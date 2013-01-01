<?php

/**
 * @file
 *
 * The delete pages form.
 */

/**
 * Gets a form that lists all the pages for the given book, given the user the
 * option to selectively delete a number of them.
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
function islandora_book_delete_pages_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $pages = islandora_book_get_pages($object);
  return array(
    'table' => array(
      '#type' => 'tableselect',
      '#header' => array(
        'pid' => t('PID'),
        'page' => t('Sequence Number'),
        'label' => t('Label'),
      ),
      '#options' => $pages,
      '#multiple' => TRUE,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Delete Selected Pages'),
    )
  );
}

/**
 * Submit handler for the delete pages form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_delete_pages_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/delete_pages.batch');
  $object = $form_state['object'];
  $pages = array_values(array_filter($form_state['values']['table']));
  batch_set(islandora_book_delete_pages_batch($object, $pages));
}
