<?php

/**
 * @file
 *
 * Handles the upload of a single page.
 */

/**
 * The form for uploading a single page object. At the moment we will always
 * append the page to the end of the book, it can be re-sequenced later.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_upload_pages_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $get_default_value = function($name, $default) use($form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : $default;
  };
  $upload_size = min((int)ini_get('post_max_size'), (int)ini_get('upload_max_filesize'));
  $extensions = array('tiff tif');
  $languages = islandora_book_get_enabled_tesseract_languages();
  // Not all uploaded pages will have a language.
  $languages['none'] = t('Not Applicable');
  // @todo Make the default language an option in the admin menu.
  $default_language = in_array('eng', array_keys($languages)) ? 'eng' : NULL;
  $default_language = $get_default_value('language', $default_language);
  return array(
    'file' => array(
      '#title' => t('Page'),
      '#type' => 'managed_file',
      '#description' => t('Select page images to upload.<br/>Files must be less than <b>!size MB.</b><br/>Allowed file types: <b>!ext.</b>', array('!size' => $upload_size, '!ext' => $extensions[0])),
      '#default_value' => $get_default_value('files', NULL),
      '#upload_location' => 'temporary://',
      '#required' => TRUE,
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
    // We present the language and preprocess options even if OCR is not enabled.
    // If the user changes their mind at a different time, we will know how to
    // generate the OCR datastreams.
    'language' => array(
      '#title' => t('Language'),
      '#type' => 'select',
      '#description' => t('Please select the language that the page is written in, if applicable. If <b>Not Applicable</b>, is selected no OCR will be generated.'),
      '#options' => $languages,
      '#default_value' => $default_language,
      '#ajax' => array(
        'callback' => 'islandora_book_upload_pages_form_language_ajax_callback',
        'wrapper' => 'preprocess-wrapper',
        'effect' => 'fade',
        'event' => 'change'
      )
    ),
    'preprocess_wrapper' => array(
      '#type' => 'markup',
      '#prefix' => '<div id="preprocess-wrapper">',
      '#suffix' => '</div>',
      'preprocess' => array(
        '#access' => $default_language != 'none',
        '#title' => t('Preprocessing for typescripts?'),
        '#type' => 'checkbox',
        '#description' => t('Will add additional processing for typewritten text. This script grayscales and sharpens the uploaded ingested TIFF before performing OCR processes.<br/>This has been shown to improve OCR results for some images. Use with caution. Process can be lengthly and is resource intensive.<br/>'),
        '#default_value' => $get_default_value('preprocess', TRUE),
      )
    )
  );
}

/**
 * Sets the appropriate datastream for the uploaded tiff page image.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_upload_pages_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = islandora_ingest_form_get_object($form_state);
  $file = file_load($form_state['values']['file']);
  $object->label = $file->filename;
  $filename = drupal_realpath($file->uri);
  islandora_book_upload_pages_form_add_update_datastream($object, 'OBJ', $filename);
  // The TIFF file is considered archival and isn't required, for future use.
  islandora_book_upload_pages_form_add_update_datastream($object, 'TIFF', $filename);
  // Set the language/preprocess options of the upload file in the RELS-INT
  $rels_int = $object['OBJ']->relationships;
  $language = $form_state['values']['language'];
  islandora_book_set_relationship($rels_int, ISLANDORA_RELS_INT_URI, 'hasLanguage', $language, TRUE);
  $preprocess = $form_state['values']['preprocess'] ? 'true' : 'false';
  islandora_book_set_relationship($rels_int, ISLANDORA_RELS_INT_URI, 'preprocessOCR', $preprocess, TRUE);
  // Update RELS-EXT properties, page/sequence/etc, and append the page at the
  // end of the book.
  $rels_ext = $object->relationships;
  $configuration = islandora_ingest_form_get_configuration($form_state);
  $book = islandora_object_load($configuration['book']);
  $pages = islandora_book_get_pages($book);
  $num_pages = count($pages);
  islandora_book_set_relationship($rels_ext, FEDORA_RELS_EXT_URI, 'isMemberOf', $book->id);
  islandora_book_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageOf', $book->id);
  islandora_book_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSequenceNumber', (string) $num_pages, TRUE);
  islandora_book_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isPageNumber', (string) $num_pages, TRUE);
  islandora_book_set_relationship($rels_ext, ISLANDORA_RELS_EXT_URI, 'isSection', '1', TRUE);
}

/**
 * Add or update the given datastream, with the given file's contents.
 *
 * @param FedoraObject $object
 *   The object that owns the created/updated datastream.
 * @param string $dsid
 *   The ID of the datastream to create/update.
 * @param string $file
 *   The absolute path to a file whose content's will be used to update the
 *   datastream.
 */
function islandora_book_upload_pages_form_add_update_datastream(NewFedoraObject $object, $dsid, $filename) {
  if (empty($object[$dsid])) {
    $ds = $object->constructDatastream($dsid, 'M');
    $object->ingestDatastream($ds);
  }
  else {
    $ds = $object[$dsid];
  }
  $ds->setContentFromFile($filename);
  $ds->label = 'Archival IMAGE';
  $ds->mimetype = 'image/tiff';
}

/**
 * Returns the renderable elemenet for the "languge" select ajax callback.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_upload_pages_form_language_ajax_callback(array $form, array $form_state) {
  return $form['preprocess_wrapper'];
}
