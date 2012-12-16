<?php

/**
 * @file
 *
 *  The menu callback to display the a books pages.
 */

/**
 * Renders the Pages local menu task.
 *
 * @param FedoraObject $object
 *   The book object to fetch the pages from.
 *
 * @return string
 *   The HTML repersentation of the given books pages.
 */
function islandora_book_pages_menu(FedoraObject $object) {
  return theme('islandora_book_pages', array('object' => $object));
}
