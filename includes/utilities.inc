<?php

/**
 * @file
 *
 * @todo Document.
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
  $query = 'select $object $page $label from <#ri>
            where $object <fedora-rels-ext:isMemberOf> <info:fedora/' . $object->id . '>
            and $object <fedora-model:label> $label
            and ($object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> $page
            or $object <http://islandora.ca/ontology/relsext#isSequenceNumber> $page)
            order by $page';
  $results = $object->repository->ri->query($query);
  $map = function($o) {
    return array(
      'label' => $o['label']['value'],
      'page' => $o['page']['value'],
      'pid' => $o['object']['value']
    );
  };
  $pages = array_map($map, $results);
  $pids = array_map(function($o) { return $o['pid']; }, $pages);
  return count($pids) ? array_combine($pids, $pages) : array();
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
