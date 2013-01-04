<?php

/**
 * @file
 *
 * Handles the generation of derivative datastreams for page objects.
 *
 * @todo Sort Language.
 */

/**
 * Generates all the derivatives for the given page object.
 *
 * @param FedoraObject $object
 *   The page object that the derivatives will be generated for.
 *
 * @return bool
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_page_create_all_derivatives(FedoraObject $object) {
  if (!isset($object['OBJ'])) {
    $msg = t('Could not create derivatives for %s. No image file was uploaded.', array(
             '%s' => $object->id));
    drupal_set_message($msg, 'error');
    return FALSE;
  }
  // Conditionally generate images, if the large image solution pack is enabled
  // and the users has enabled local generation of image datastreams.
  $generated_images = TRUE;
  if (variable_get('islandora_book_create_images', TRUE) && module_exists('islandora_large_image')) {
    module_load_include('inc', 'islandora_large_image', 'includes/islandora_large_image.process');
    $generated_images = islandora_large_image_create_all_derivatives($object);
  }
  // Conditionally generate images, if the users has enabled local generation of ocr datastreams.
  $generated_ocr = TRUE;
  if (variable_get('islandora_book_do_ocr', TRUE)) {
    $generated_ocr = islandora_book_page_create_ocr_derivatives($object, TRUE);
  }
  return $generated_images && $generated_ocr;
}

/**
 * Creates the OCR based datastreams from the uploaded image file.
 *
 * @param FedoraObject $object
 *   The page object that the derivative will be generated for.
 * @param boolean $preprocess
 *   Attempt to preprocess the image before generating the OCR.
 *
 * @return boolean
 *   TRUE if successfule, FALSE otherwise.
 */
function islandora_book_page_create_ocr_derivatives(FedoraObject $object, $preprocess = FALSE) {
  $filename = islandora_book_page_get_derivative_source_file($object, $preprocess);
  $generated_ocr = islandora_book_page_create_ocr_derivative($object, $filename);
  $generated_hocr = islandora_book_page_create_hocr_derivatives($object, $filename);
  return $generated_ocr && $generated_hocr;
}

/**
 * Gets the uploaded image file for the given page object, will attempt to
 * preprocess (Generate a grayscale) of the image if instructed.
 *
 * @param FedoraObject $object
 *   The page object to get the uploaded file from.
 * @param boolean $preprocess
 *   Attempt to preprocess the image.
 *
 * @return string
 *   A absolute path to the uploaded image file. The caller is responsible for
 *   deleting this file.
 */
function islandora_book_page_get_derivative_source_file(FedoraObject $object, $preprocess = FALSE) {
  $mime_detector = new MimeDetect();
  $ext = $mime_detector->getExtension($object['OBJ']->mimeType);
  $filename = drupal_realpath("temporary://{$base_name}_OBJ.{$ext}");
  $object['OBJ']->getContent($filename);
  if ($preprocess && variable_get('islandora_book_ocr_do_preprocess', FALSE)) {
    // @todo check for gimp via admin setting
    $executable = variable_get('islandora_book_gimp_path', '/usr/bin/gimp');
    $command = "$executable -d -i -b '(munge-typeset-ocr {$filename} {$filename}.png 5.0 10.0 0.0 200.0 5.0)' -b '(gimp-quit 0)'";
    exec($command, $output, $ret);
    if ($ret == 0) {
      file_unmanaged_delete($filename);
      return "$filename.png";
    }
  }
  return $filename;
}

/**
 * Creates the OCR data stream for the given page object.
 *
 * @param FedoraObject $object
 *   The page object that the derivative will be generated for.
 * @param string $filename
 *   The absolute path to the uploaded image file used to generate the OCR.
 *
 * @return boolean
 *   TRUE if successfule, FALSE otherwise.
 */
function islandora_book_page_create_ocr_derivative(FedoraObject $object, $filename) {
  $executable = variable_get('islandora_book_ocr_path', '/usr/bin/tesseract');
  $output = $ret = '';
  exec("$executable $filename $filename -l $language", $output, $ret);
  // Generated file from exec.
  $filename = "$filename.txt";
  $successful = ($ret == '0');
  if (!$successful) {
    // Default value for data stream when OCR generation fails.
    file_put_contents($filename, 'No OCR availible');
  }
  $added_datastream = islandora_book_page_update_derived_datastream($object, 'OCR', 'Scanned Text', 'text/plain', $filename);
  file_unmanaged_delete($filename);
  return $successful && $added_datastream;
}

/**
 * Creates the RAW_HOCR and ENCODED_OCR data streams for the given page object.
 *
 * @param FedoraObject $object
 *   The page object that the derivative will be generated for.
 * @param string $filename
 *   The absolute path to the uploaded image file used to generate the OCR.
 *
 * @return boolean
 *   TRUE if successfule, FALSE otherwise.
 */
function islandora_book_page_create_hocr_derivatives(FedoraObject $object, $filename) {
  $executable = variable_get('islandora_book_ocr_path', '/usr/bin/tesseract');
  exec("$executable $filename $filename -l $language hocr", $output, $ret);
  if ($ret != '0') {
    return FALSE;
  }
  $hocr_filename = "$filename.html";
  $added_raw_hocr_datastream = islandora_book_page_update_derived_datastream($object, 'RAW_HOCR', 'RAW_HOCR', 'text/xml', $hocr_filename);
  $encoded_hocr_filename = islandora_book_page_convert_hocr_html_to_lower_xml($hocr_filename);
  $added_encoded_hocr_datastream = islandora_book_page_update_derived_datastream($object, 'ENCODED_OCR', 'ENCODED_OCR', 'text/xml', $encoded_hocr_filename);
  file_unmanaged_delete($hocr_filename);
  file_unmanaged_delete($encoded_hocr_filename);
  return $added_raw_hocr_datastream && $added_encoded_hocr_datastream;
}

/**
 * Converts the given hocr html file to a lowercase xml file.
 *
 * @param string $filename
 *   The absolute path to the hocr html file.
 *
 * @return string
 *   The absolute path to the hocr lowercase xml file
 */
function islandora_book_page_convert_hocr_html_to_lower_xml($filename) {
  $proc = new XSLTProcessor();
  // Convert the hocr from html 4.01 transitional to xhtml and lower case
  // the text.
  $html_string = file_get_contents($filename);
  $doc_type = '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">';
  $xml_string = str_replace($doc_type, '<?xml version="1.0" encoding="UTF-8"?>', $html_string);
  $input = new DOMDocument();
  $input->loadXML($xml_string);
  $path = drupal_get_path('module', 'islandora_book');
  $xsl = new DOMDocument();
  $xsl->load($path . '/xsl/hocrToLowerhocr.xsl');
  $xsl = $proc->importStylesheet($xsl);
  $result = $proc->transformToDoc($input);
  // Create new transformed file.
  $filename = "$filename.xml";
  $result->save($filename);
  return $filename;
}

/**
 * Adds/updates a given datastream properties/content with the given parameters.
 *
 * @param FedoraObject $object
 *   The page object that the datastream will be added to.
 * @param string $dsid
 *   The datastream ID.
 * @param string $label
 *   The datastream label.
 * @param string $mimetype
 *   The datastream mimetype.
 * @param string $file
 *   The absolute path to the file containing the datastream content.
 *
 * @return boolean
 *   TRUE if successful, FALSE otherwise.
 */
function islandora_book_page_update_derived_datastream(FedoraObject $object, $dsid, $label, $mimetype, $file) {
  if (empty($object[$dsid])) {
    $ds = $object->constructDatastream($dsid, 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object[$dsid];
  }
  $ds->label = $label;
  $ds->mimeType = $mimetype;
  $ds->setContentFromFile($file);
  try {
    $object->ingestDatastream($ds);
  } catch (exception $e) {
    drupal_set_message(t('@message', array('@message' => check_plain($e->getMessage()))), 'error');
    return FALSE;
  }
  return TRUE;
}
