<?php

/**
 * @file
 *
 */

/**
 *
 */
function islandora_book_upload_pages_form(array $form, array &$form_state) {
  $upload_size = min((int)ini_get('post_max_size'), (int)ini_get('upload_max_filesize'));
  $extensions = array('tiff tif');
  return array(
    'file' => array(
      '#title' => t('Pages'),
      '#type' => 'manage_file',
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
 *
 */
function islandora_book_upload_pages_form_submit(array $form, array &$form_state) {

}
