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
  $rows = array_map(function($page) {
            array(
              'pid' => array('#markup' => check_plain($page['pid'])),
              'label' => array('#markup' => check_plain($page['pid'])),
              'pid' => array('#markup' => check_plain($page['pid'])),
            );
          }, $pages);
  return array(
    '#tree' => TRUE,
    'table' => array(
      '#type' => 'markup',
      '#header' => array(
        'pid' => t('PID'),
        'page' => t('Sequence Number'),
        'label' => t('Label'),
      ),
      '#options' => $pages,
      '#multiple' => TRUE,
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
 * Submit handler for the reorder pages form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_reorder_pages_form_submit(array $form, array &$form_state) {
  $pages = array_filter($form_state['values']['table']);
  // @todo Should be a batch process cause it could take too long.
  array_map('islandora_delete_object', $pages);
}
