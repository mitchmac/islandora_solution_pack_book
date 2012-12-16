<?php

/**
 * @file
 *
 * Provides the menu callback and form for ingesting pages.
 */

/**
 * Gets the book ingest pages form.
 *
 * @param FedoraObject $object.
 *   The collection to ingest into.
 *
 * @return sting
 *   The HTML repersentation of the ingest pages form.
 */
function islandora_book_ingest_pages(FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  return drupal_get_form('islandora_ingest_form', array(
      'book' => $object->id,
      'models' => array('islandora:pageCModel'),
    )
  );
}
