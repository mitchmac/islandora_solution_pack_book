<?php

/**
 * @file
 *
 * Handles the upload of a single page.
 */

/**
 * The form for uploading a single page object.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_upload_pages_form(array $form, array &$form_state) {
  $upload_size = min((int)ini_get('post_max_size'), (int)ini_get('upload_max_filesize'));
  $extensions = array('tiff tif');
  return array(
    'file' => array(
      '#title' => t('Page'),
      '#type' => 'managed_file',
      '#description' => t('Select page images to upload.<br/>Files must be less than <b>!size MB.</b><br/>Allowed file types: <b>!ext.</b>', array('!size' => $upload_size, '!ext' => $extensions[0])),
      '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
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
  $object = islandora_ingest_form_get_object($form_state);
  $file = file_load($form_state['values']['file']);
  $filename = drupal_realpath($file->uri);
  islandora_book_upload_pages_form_add_update_datastream($object, 'OBJ', $filename);
  islandora_book_upload_pages_form_add_update_datastream($object, 'TIFF', $filename);
  // @todo Deal with these relationships.
  // @todo Deal with the ordering of pages.
  // isSequenceNumber http://islandora.ca/ontology/relsext#
  // isPageNumber http://islandora.ca/ontology/relsext#
  // isPageOf http://islandora.ca/ontology/relsext#
  // isSection '1' http://islandora.ca/ontology/relsext#
  // isMemberOf RELS_EXT_URI
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
function islandora_book_upload_pages_form_add_update_datastream(FedoraObject $object, $dsid, $filename) {
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
