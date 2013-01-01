<?php

/**
 * @file
 *
 * The book management menu.
 */

/**
 * Displays all the book management forms in a set of vertical tabs.
 *
 * @param FedoraObject $object
 *   The book object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   book objects.
 */
function islandora_book_manage_menu(FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/update_derived_datastreams.form');
  module_load_include('inc', 'islandora_book', 'includes/reorder_pages.form');
  module_load_include('inc', 'islandora_book', 'includes/delete_pages.form');
  return array(
    'manage_book' => array(
      '#type' => 'vertical_tabs',
      '#weight' => 0,
      '#prefix' => '',
      /*'update_datastreams' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Update Derived Datastreams'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_update_derived_datastreams_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        ),*/
      'reorder_pages' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Reorder Pages from this Book'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_reorder_pages_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'delete_pages' => array(
        // @todo Replace with check for permission.
        '#access' => user_access(PERM_ISLANDORA_BOOK_DELETE_PAGES),
        '#title' => t('Delete Pages from this Book'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_delete_pages_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      )
    )
  );
}
