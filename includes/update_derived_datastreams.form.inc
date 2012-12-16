<?php

/**
 * @file
 *
 * The derive datastreams form and its batch operations.
 */


/**
 * Gets a form that allows the user to update the derived datastreams of all
 * the page objects that belong to the given book object.
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
function islandora_book_update_derived_datastreams_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $default_value = function($field, $default) use($form_state) {
    return isset($form_state['values'][$field]) ? $form_state['values'][$field] : $default;
  };
  $form_state['object'] = $object;
  $languages = array_filter(variable_get('islandora_book_ocr_languages'));
  $languages = array_intersect_key(islandora_book_get_tesseract_langauges(), $languages);
  return array(
    'images' => array(
      '#type' => 'checkbox',
      '#title' => t('Refresh all derived images'),
      '#default_value' => $default_value('images', FALSE)
    ),
    'pdf' => array(
      '#type' => 'checkbox',
      '#title' => t('Generate a PDF of This Book'),
      '#description' => t('The PDF will be added as a datastream to this object.'),
      '#default_value' => $default_value('pdf', FALSE)
    ),
    'ocr' => array(
      '#type' => 'checkbox',
      '#title' => t('Refresh OCR for this book'),
      '#default_value' => $default_value('ocr', FALSE)
    ),
    'language' => array(
      '#type' => 'select',
      '#title' => t('Language'),
      '#options' => $languages,
      '#default_value' => $default_value('language', NULL)
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update Datastreams'),
    )
  );
}

/**
 * Submit handler for the update derived datastreams form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_update_derived_datastreams_form_submit(array $form, array &$form_state) {
  $values = $form_state['values'];
  $object = $form_state['object'];
  $pages = array_values(islandora_book_get_pages($object));
  $create_op = function($op) use($object, $pages) { return array($op, array($object, $pages)); };
  $operations = array_filter(
    array(
      $values['images'] ? $create_op('islandora_book_derive_images_batch_operation') : NULL,
      $values['ocr'] ? $create_op('islandora_book_derive_text_batch_operation') : NULL,
      $values['pdf'] ? $create_op('islandora_book_derive_pdf_batch_operation') : NULL
    )
  );
  $batch = array(
    'operations' => $operations,
    'finished' => 'islandora_book_update_derived_datastreams_batch_finished',
    'title' => t('Updating derived datastreams'),
    'init_message' => t('Preparing to update derived datastreams.'),
    'progress_message' => t('Updated @current out of @total.'),
    'error_message' => t('An error has occured.'),
    'file' => drupal_get_path('module', 'islandora_book') . '/includes/update_derived_datastreams.form.inc',
  );
  batch_set($batch);
}

/**
 * Sets up a derived datastreams batch operation.
 *
 * @param FedoraObject $object
 *   The book object.
 * @param array $pages
 *   All the pages to update.
 * @param array $context
 *   The context of this batch operation.
 *
 * @return array $page
 *   The page to operate on.
 */
function islandora_book_update_derived_datastreams_batch_start_operation(FedoraObject $object, array $pages, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($pages);
  }
  $page_index = $context['sandbox']['progress'];
  return $pages[$page_index];
}

/**
 * Ends a derived datastreams batch operation.
 *
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_update_derived_datastreams_batch_end_operation(array &$context) {
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
}

/**
 * Derives the image datastreams for the given page objects.
 *
 * @param FedoraObject $object
 *   The book object.
 * @param array $pages
 *   All the pages to update.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_derive_images_batch_operation(FedoraObject $object, array $pages, array &$context) {
  $page = islandora_book_update_derived_datastreams_batch_start_operation($object, $pages, $context);
  $context['message'] = t('Generating image derivatives for %label: (%pid)', array(
                          '%label' => $page['label'],
                          '%pid' => $page['pid']));
  // @todo Actually generate the images.
  sleep(1);
  $context['results'][] = "{$page['label']}: ({$page['pid']})";
  islandora_book_update_derived_datastreams_batch_end_operation($context);
}

/**
 * Derives the text datastreams for the given page objects.
 *
 * @param FedoraObject $object
 *   The book object.
 * @param array $pages
 *   All the pages to update.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_derive_text_batch_operation(FedoraObject $object, array $pages, array &$context) {
  $page = islandora_book_update_derived_datastreams_batch_start_operation($object, $pages, $context);
  $context['message'] = t('Generating text derivatives for %label: (%pid)', array(
                          '%label' => $page['label'],
                          '%pid' => $page['pid']));
  // @todo Actually generate the images.
  sleep(1);
  $context['results'][] = "{$page['label']}: ({$page['pid']})";
  islandora_book_update_derived_datastreams_batch_end_operation($context);
}

/**
 * Generates a PDF from all the given page objects.
 *
 * @param FedoraObject $object
 *   The book object.
 * @param array $pages
 *   All the pages to combined into a PDF.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_book_derive_pdf_batch_operation(FedoraObject $object, array $pages, array &$context) {
  $page = islandora_book_update_derived_datastreams_batch_start_operation($object, $pages, $context);
  $context['message'] = t('Generating PDF from page %label: (%pid)', array(
                          '%label' => $page['label'],
                          '%pid' => $page['pid']));
  // @todo Actually generate the images.
  sleep(1);
  $context['results'][] = "{$page['label']}: ({$page['pid']})";
  islandora_book_update_derived_datastreams_batch_end_operation($context);
}

/**
 * The 'finished' callback for the derived datastreams batch.
 */
function islandora_book_update_derived_datastreams_batch_finished($success, $results, $operations) {
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
