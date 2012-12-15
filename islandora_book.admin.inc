<?php

/**
 * @file
 *
 * Handles the display/submission of the admin settings form for this module.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_book_admin_settings_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'book_pack_utils');
  $get_default_value = function($name, $default) use($form_state) {
    return isset($form_state['values'][$name]) ? $form_state['values'][$name] : variable_get($name, $default);
  };
  $do_ocr = $get_default_value('islandora_book_do_ocr', TRUE);
  $ocr_path = $get_default_value('islandora_book_ocr_path', '/usr/bin/tesseract');
  $ocr_availible_message = islandora_book_admin_settings_form_ocr_availible_message($ocr_path);
  $form = array(
    'islandora_book_solr_subject_field' => array(
      '#type' => 'textfield',
      '#title' => t('Solr subject field'),
      '#description' => t('The solr field containing your untokenized subject value, used to link between books with the same subject.'),
      '#default_value' => $get_default_value('islandora_book_solr_subject_field', 'mods.subject'),
    ),
    'islandora_book_create_images' => array(
      '#type' => 'checkbox',
      '#title' => t('Create derivative images locally?'),
      '#description' => t('Check this box to have images produced locally on ingest.'),
      '#default_value' => $get_default_value('islandora_book_create_images', TRUE),
    ),
    'islandora_book_create_pdf' => array(
      '#type' => 'checkbox',
      '#title' => t('Create derivative PDF locally?'),
      '#description' => t('Check this box to have a PDF produced locally on ingest.'),
      '#default_value' => $get_default_value('islandora_book_create_pdf', FALSE),
    ),
    'islandora_book_do_ocr' => array(
      '#type' => 'checkbox',
      '#title' => t('Perform OCR on incoming TIFF images?'),
      '#description' => t('Do not check this box if OCR is performed on an external server, such as ABBYY'),
      '#default_value' => $do_ocr,
      '#ajax' => array(
        'callback' => 'islandora_book_admin_settings_form_use_ocr_ajax_callback',
        'wrapper' => 'do-ocr-fields-wrapper',
        'effect' => 'fade',
        'event' => 'change'
      )
    ),
    'do_ocr_fields_wrapper' => array (
      '#prefix' => '<div id="do-ocr-fields-wrapper">',
      '#suffix' => '</div>',
      'do_ocr_fields' => array (
        '#type' => 'fieldset',
        '#title' => t('OCR Settings'),
        '#access' => $do_ocr,
        'islandora_book_ocr_languages' => array(
          '#type' => 'checkboxes',
          '#title' => t('Languages available for OCR'),
          '#options' => islandora_book_get_tesseract_langauges(),
          '#description' => t('Select from the language packs available on your
                             processing server.<br/> These are normally found in
                             <strong>/usr/local/share/tessdata/</strong><br/>
                             Check with your systems admininistrator if you are
                             unsure of availablilty'),
          '#default_value' => islandora_book_get_available_tesseract_languages(),
        ),
        'islandora_book_ocr_path' => array(
          '#type' => 'textfield',
          '#prefix' => '<div id="ocr-path-wrapper">',
          '#suffix' => '</div>',
          '#title' => t('Path to OCR executable'),
          '#description' => t('Path to OCR program on your server.<br/>!msg', array('!msg' => $ocr_availible_message)),
          '#default_value' => $ocr_path,
          '#ajax' => array(
            'callback' => 'islandora_book_admin_settings_form_ocr_path_ajax_callback',
            'wrapper' => 'ocr-path-wrapper',
            'effect' => 'fade',
            'event' => 'change'
          )
        )
      )
    )
  );
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  $form += islandora_viewers_form('islandora_book_viewers', NULL, 'islandora:iaBookCModel');
  return system_settings_form($form);
}

/**
 * Gets a message to display if the ocr program provided is accessible or not.
 *
 * @todo Add additional checks to see if the program is executable.
 * @todo Also check if the return value for 'ls' will work for all systems.
 *
 * @param string $ocr_path
 *   The path to an executable responsible for generating ocr data.
 *
 * @return string
 *   A message in html detailing if the given executable is accessible.
 */
function islandora_book_admin_settings_form_ocr_availible_message($ocr_path) {
  $ocr_availible = exec("ls $ocr_path");
  if ($ocr_availible) {
    $image = theme_image(array('path' => 'misc/watchdog-ok.png', 'attributes' => array()));
    $message = t('Executable found at @ocr_path', array('@ocr_path' => $ocr_path));
  }
  else {
    $image = theme_image(array('path' => 'misc/watchdog-error.png', 'attributes' => array()));
    $message = t('Unable to locate executable at @ocr_path', array('@ocr_path' => $ocr_path));
  }
  return $image . $message;
}

/**
 * Ajax callback for the "ocr" checkbox field
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_book_admin_settings_form_use_ocr_ajax_callback(array $form, array $form_state) {
  return $form['do_ocr_fields_wrapper'];
}

/**
 * Ajax callback for the "ocr_path" textfield.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The element to render as part the ajax callback.
 */
function islandora_book_admin_settings_form_ocr_path_ajax_callback(array $form, array $form_state) {
  return $form['do_ocr_fields_wrapper']['do_ocr_fields']['islandora_book_ocr_path'];
}
