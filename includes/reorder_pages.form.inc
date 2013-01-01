<?php

/**
 * @file
 *
 * The page reorder pages form.
 */

/**
 * Gets a form that lists all the pages for the given book, given the user the
 * option to reorder them.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The drupal form.
 */
function islandora_book_reorder_pages_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $pages = islandora_book_get_pages($object);
  $delta = count($pages);
  $rows = array_map(function($page) use($delta) {
            return array(
              'pid' => array('#markup' => check_plain($page['pid'])),
              'label' => array('#markup' => check_plain($page['label'])),
              'weight' => array(
                '#type' => 'weight',
                '#title' => t('Weight'),
                '#default_value' => $page['page'],
                '#delta' => $delta,
                '#title-display' => 'invisible',
              ));
          }, $pages);
  return array(
    '#tree' => TRUE,
    'table' => array(
      '#type' => 'markup',
      '#header' => array(t('PID'), t('Label'), t('Page')),
      'rows' => $rows,
    ),
    'actions' => array(
      '#type' => 'actions',
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Save Changes')
      )
    )
  );
}

/**
 * Implements theme_form_id().
 */
function theme_islandora_book_reorder_pages_form($variables) {
  $form = $variables['form'];
  $rows = array();
  $weight_class = 'islandora-book-page-weight';
  $table = &$form['table'];
  foreach (element_children($table['rows']) as $id) {
    $row = &$table['rows'][$id];
    $row['weight']['#attributes'] = array('class' => array($weight_class));
    $rows[] = array(
      'data' => array(
        drupal_render($row['pid']),
        drupal_render($row['label']),
        drupal_render($row['weight']),
      ),
      'class' => array('draggable'),
    );
  }
  $table_id = 'islandora-reorder-pages-table';
  $output = theme('table', array('header' => $table['#header'], 'rows' => $rows, 'attributes' => array('id' => $table_id)));
  $output .= drupal_render_children($form);
  drupal_add_tabledrag($table_id, 'order', 'sibling', $weight_class);
  return $output;
}

/**
 * Submit handler for the reorder pages form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_reorder_pages_form_submit(array $form, array &$form_state) {
  $pages = &$form_state['values']['table']['rows'];
  uasort($pages, 'drupal_sort_weight');
  $pages = array_keys($pages);
  $batch = array(
    'operations' => array(
      array('islandora_book_reorder_pages_batch_operation', array($pages))
    ),
    'finished' => 'islandora_book_reorder_pages_batch_finished',
    'title' => t('Reordering pages'),
    'init_message' => t('Preparing to reorder pages.'),
    'progress_message' => t('Updated @current out of @total.'),
    'error_message' => t('An error has occured.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/reorder_pages.form.inc',
  );
  batch_set($batch);
}

/**
 * Changes the ordering of the pages.
 *
 * @param array $pages
 *   All the pages in the order they should be in.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_reorder_pages_batch_operation(array $pages, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($pages);
  }
  $page_index = $context['sandbox']['progress'];
  $page = $pages[$page_index];
  $object = islandora_object_load($page);
  if ($object) {
    $context['message'] = t('Inserting page %label: (%pid) at position "%pos"', array(
                            '%label' => $object->label,
                            '%pid' => $object->id,
                            '%pos' => $page_index));
    // Remove old predicate.
    $object->relationships->remove('info:islandora/islandora-system:def/paged-item-info#', 'sequenceNumber');
    $results = $object->relationships->get('http://islandora.ca/ontology/relsext#', 'isSequenceNumber');
    if (isset($results[0]['object']['value'])) {
      // Only change if its different than before.
      if ($results[0]['object']['value'] != $page_index) {
        $object->relationships->remove('http://islandora.ca/ontology/relsext#', 'isSequenceNumber');
        $object->relationships->add('http://islandora.ca/ontology/relsext#', 'isSequenceNumber', $page_index);
      }
    }
    $context['results'][] = "{$object->label}: ({$object->id}) inserted at {$page_index}";
  }
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
}

/**
 * The 'finished' callback for the derived datastreams batch.
 */
function islandora_book_reorder_pages_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = count($results) .' processed.';
    $message .= theme('item_list', $results);
    drupal_set_message($message);
  }
  else {
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
                 '%error_operation' => $error_operation[0],
                 '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}
