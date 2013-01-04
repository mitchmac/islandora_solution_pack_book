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
  $pages = count($pids) ? array_combine($pids, $pages) : array();
  uasort($pages, function($a, $b) {
      $a_page = (is_array($a) && isset($a['page'])) ? $a['page'] : 0;
      $b_page = (is_array($b) && isset($b['page'])) ? $b['page'] : 0;
      if ($a_page == $b_page) {
        return 0;
      }
      return ($a_page < $b_page) ? -1 : 1;
    });
  return $pages;
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


/**
 * Gets the available languages that tesseract supports.
 *
 * @todo Generate these values by looking at the tesseract install location.
 *
 * @return array
 *   The supported languages where the key is the value tesseract recognizes and
 *   the value is a human readable version of the language.
 */
function islandora_book_get_tesseract_langauges() {
  return array(
    'eng' => t('English'),
    'fra' => t('French'),
    'deu-frak' => t('German'),
    'por' => t('Portugese'),
    'spa' => t('Spanish'),
    'hin' => t('Hindi'),
    'jpn' => t('Japanese'),
    'ita' => t('Italian')
  );
}

/**
 * Gets only the enabled languages as defined by the user.
 *
 * @return array
 *   The enabled languages where the key is the value tesseract recognizes and
 *   the value is a human readable version of the language.
 */
function islandora_book_get_enabled_tesseract_langauges() {
  $tesseract_languages = islandora_book_get_tesseract_langauges();
  $enabled_languages = array_filter(variable_get('islandora_book_ocr_languages', $tesseract_languages));
  return array_intersect_key($tesseract_languages, $enabled_languages);
}
