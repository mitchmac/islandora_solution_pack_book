<?php

/**
 * @file
 *
 */

/**
 * Gets all the pages in the given book object.
 *
 * @param FedoraObject $object
 *   The book object to fetch the pages from.
 *
 * @return array
 *   All the pages in the given book where the key values are the page numbers
 *   and the values are the PIDs of the page objects.
 */
function islandora_book_get_pages(FedoraObject $object) {
  $query = 'select $object $page from <#ri>
            where $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $object->id . '>
            and ($object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> $page
            or $object <http://islandora.ca/ontology/relsext#isSequenceNumber> $page)
            order by $page';
  $results = $object->repository->ri->query($query);
  $page_numbers = array_map(function($o) { return $o['page']['value']; }, $results);
  $page_pids = array_map(function($o) { return $o['object']['value']; }, $results);
  return array_combine($page_numbers, $page_pids);
}

/**
 * Gets the page progression to be used in a viewer from the given book.
 *
 * @param FedoraObject $object
 *   The book object to fetch the page progression from.
 *
 * @return string
 *   The page progression of the book either left to right 'lr' or
 *   right to left 'rl'.
 */
function islandora_book_get_page_progression(FedoraObject $object) {
  $query = 'select $page_progression from <#ri> where <info:fedora/' . $object->id . '>
            <fedora-rels-ext:hasPageProgression> $page_progression';
  $results = $object->repository->ri->query($query);
  if (count($results) > 0) {
    return $results[0]['page_progression']['value'];
  }
  return 'lr';
}
