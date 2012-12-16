<?php

/**
 * @file
 *
 */

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_book(array &$variables) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $variables['object'];
  $params = array(
    'object' => $object,
    'pages' => array_keys(islandora_book_get_pages($object)),
    'page_progression' => islandora_book_get_page_progression($object),
  );
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $viewer = islandora_get_viewer($params, 'islandora_book_viewers');
  if ($viewer) {
    $variables['viewer'] = $viewer;
  }
}

/**
 * Implements hook_preprocess_theme().
 */
function islandora_book_preprocess_islandora_book_pages(array &$variables) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $object = $variables['object'];
  // View Links
  $display = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $grid_active = ($display == 'grid') ? 'active' : '';
  $list_active = ($display == 'active') ? 'active' : '';
  $query_params = drupal_get_query_parameters($_GET);
  $variables['view_links'] = array(
    array(
      'title' => 'Grid view',
      'href' => url("islandora/object/{$object->id}/pages", array('absolute' => TRUE)),
      'attributes' => array(
        'class' => "islandora-view-grid $grid_active"
      ),
      'query' => $query_params + array('display' => 'grid')
    ),
    array(
      'title' => 'List view',
      'href' => url("islandora/object/{$object->id}/pages", array('absolute' => TRUE)),
      'attributes' => array(
        'class' => "islandora-view-list $list_active"
      ),
      'query' => $query_params + array('display' => 'list')
    )
  );
  // Pager
  $pages = islandora_book_get_pages($object);
  $page_size = (empty($_GET['pagesize'])) ? 20 : $_GET['pagesize'];
  $total_count = count($pages);
  pager_default_initialize($total_count, $page_size);
  $variables['pager'] = theme('pager', array('quantity' => 10));
  // Content
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $start = $page_size * ($page_number);
  $end = min($start + $page_size, $total_count);
  // Only display current page.
  $pages = array_splice($pages, $start, $end);
  $pages = array_map(function($o) {
             $object_url = "islandora/object/{$o['pid']}";
             $img = theme_image(array('path' => url("$object_url/datastream/TN/view"), 'attributes' => array()));
             $dc = DublinCore::import_from_xml_string(islandora_datastream_load('DC', $o['pid'])->content);
             return $o + array(
               'class' => drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $o['pid'])),
               'link' => l($o['label'], $object_url, array('html' => TRUE, 'attributes' => array('title' => $o['label']))),
               'image' => l($img, $object_url, array('html' => TRUE, 'attributes' => array('title' => $o['label']))),
               'DC' => $dc->as_formatted_array()
             );
           }, $pages);
  $theme = ($display == 'grid') ? 'islandora_book_pages_grid' : 'islandora_book_pages_list';
  $variables['content'] = theme($theme, array('object' => $object, 'pages' => $pages));
  $module_path = drupal_get_path('module', 'islandora_book');
  drupal_add_css("$module_path/css/islandora_book.css");
}
