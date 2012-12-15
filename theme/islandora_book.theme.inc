<?php

/**
 * @file
 *
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_book_view(array &$variables) {
  $object = $variables['object'];
  // Get token to allow access to XACML protected datastreams if necessary.
  $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view", array('absolute' => TRUE));
  if (isset($object['POLICY'])) {
    module_load_include('inc', 'islandora', 'includes/islandora_authtokens');
    $token = islandora_get_object_token($object->id, 'JP2', 2);
    $jp2_url = url("islandora/object/{$object->id}/datastream/JP2/view",
               array(
                 'absolute' => TRUE,
                 'query' => array('token'=>$token),
               ));
  }
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $viewer = islandora_get_viewer($jp2_url, 'islandora_large_image_viewers');
}
