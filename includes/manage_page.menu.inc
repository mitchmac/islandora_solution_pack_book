<?php

/**
 * @file
 *
 * The page management menu.
 */

/**
 * Displays all the page management forms in a set of vertical tabs.
 *
 * @param FedoraObject $object
 *   The page object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   book objects.
 */
function islandora_book_page_manage_menu(FedoraObject $object) {
  return array(
    'manage_page' => array(
      '#type' => 'vertical_tabs',
      '#weight' => 0,
      '#prefix' => '',
      'update_ocr' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Update OCR'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_update_page_ocr_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      ),
      'update_images' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Update Images'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_update_page_images_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'update_pdf' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Update PDF'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_update_page_pdf_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
    )
  );
}

/**
 * Updates this objects derived OCR datastreams.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The page object whose OCR datastreams will be updated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_update_page_ocr_form(array $form_state, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $languages = islandora_book_get_enabled_tesseract_languages();
  $rels_int = $object['OBJ']->relationships;
  $default_language = islandora_book_get_relationship($rels_int, ISLANDORA_RELS_INT_URI, 'hasLanguage');
  $default_language = in_array($default_language, array_keys($languages)) ? $default_language : NULL;
  $default_preprocess = islandora_book_get_relationship($rels_int, ISLANDORA_RELS_INT_URI, 'preprocessOCR');
  $can_update_ocr = isset($object['OBJ']) && islandora_book_can_generate_ocr();
  $can_preprocess_ocr = islandora_book_can_preprocess_ocr();
  return array(
    'language' => array(
      '#title' => t('Language'),
      '#type' => 'select',
      '#description' => t('Please select the language that the page is written in.'),
      '#options' => $languages,
      '#default_value' => $default_language,
    ),
    'preprocess' => array(
        '#disabled' => !$can_preprocess_ocr,
        '#title' => t('Preprocessing for typescripts?'),
        '#type' => 'checkbox',
        '#description' => t('Will add additional processing for typewritten text. This script grayscales and sharpens the archival TIFF before performing OCR processes.<br/>This has been shown to improve OCR results for some images. Use with caution. Process can be lengthly and is resource intensive.<br/>'),
        '#default_value' => $can_preprocess_ocr && $default_preprocess,
    ),
    'submit' => array(
      '#disabled' => !$can_update_ocr,
      '#type' => 'submit',
      '#value' => t('Update OCR'),
    )
  );
}

/**
 * Submit handler for the update page OCR form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_update_page_ocr_form_submit(array $form_state, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/page.process');
  $object = $form_state['object'];
  $language = $form_state['values']['language'];
  $preprocess = (bool) $form_state['values']['preprocess'];
  islandora_book_page_create_ocr_derivatives($object, $language, $preprocess);
}

/**
 * Updates this objects derived image datastreams.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The page object whose image datastreams will be updated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_update_page_images_form(array $form_state, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_generate_images = islandora_book_can_generate_images();
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>Large Image Solution Pack</b> installed to update images.'),
    ),
    'submit' => array(
      '#disabled' => !$can_generate_images,
      '#type' => 'submit',
      '#value' => t('Update Images'),
    )
  );
}

/**
 * Submit handler for the update page images form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_update_page_images_form_submit(array $form_state, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/page.process');
  $object = $form_state['object'];
  islandora_book_page_create_image_derivatives($object);
}

/**
 * Updates this objects pdf datastreams.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The page object whose image datastreams will be updated.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_update_page_pdf_form(array $form_state, array &$form_state, FedoraObject $object) {
  $can_generate_pdf = islandora_book_can_create_pdf();
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>ImageMagick</b> installed to update PDF files.'),
    ),
    'submit' => array(
      '#disabled' => !$can_generate_pdf,
      '#type' => 'submit',
      '#value' => t('Update PDF'),
    )
  );
}

/**
 * Submit handler for the update page images form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_update_page_pdf_form_submit(array $form_state, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/page.process');
  $object = $form_state['object'];
  islandora_book_page_create_pdf_derivatives($object);
}
