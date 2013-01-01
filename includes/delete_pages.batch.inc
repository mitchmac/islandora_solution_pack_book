<?php

/**
 * @file
 *
 * The delete pages batch.
 */

/**
 * Gets the batch definition for deleting the given pages from the given book.
 *
 * @param FedoraObject $object
 *   The object the pages belong to.
 * @param array $pages
 *   All the pages to delete.
 *
 * @return array
 *   The batch definition.
 */
function islandora_book_delete_pages_batch(FedoraObject $object, array $pages) {
  return array(
    'operations' => array(
      array('islandora_book_delete_page_batch_operation', array($pages))
    ),
    'finished' => 'islandora_book_delete_pages_batch_finished',
    'title' => t('Deleting page(s) from @label', array(
               '@label' => $object->label)
    ),
    'init_message' => t('Preparing to delete page(s) from @label.', array(
                      '@label' => $object->label)
    ),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occured.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/delete_pages.batch.inc',
  );
}

/**
 * Batch operation for deleting a page object.
 *
 * @param array $pages
 *   All the pages to delete.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_delete_page_batch_operation(array $pages, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($pages);
  }
  $page_index = $context['sandbox']['progress'];
  $object = islandora_object_load($pages[$page_index]);
  $context['message'] = t('Deleting page "@label": (@pid)', array(
                          '@label' => $object->label,
                          '@pid' => $object->id));
  if (islandora_delete_object($object)) {
    $context['results'][] = t('Deleted "@label": (@pid)', array(
                              '@label' => $object->label,
                              '@pid' => $object->id));
  }
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
}

/**
 * The 'finished' callback for the delete pages batch.
 */
function islandora_book_delete_pages_batch_finished($success, $results, $operations) {
  if ($success) {
    $message = count($results) .' pages deleted.';
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
