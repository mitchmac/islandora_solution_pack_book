<?php

/**
 * @file
 *
 * Contains one local action for ingesting pages, and one local task which
 * contains several forms for managing book objects and their pages.
 *
 * Management Forms:
 *  Create PDF.
 *  Create OCR.
 *  Create Images.
 *  Reorder Pages.
 *  Delete Pages.
 */

/**
 * Local menu action, that gets the ingest page form.
 *
 * @param FedoraObject $object.
 *   The collection to ingest into.
 *
 * @return sting
 *   The HTML repersentation of the ingest page form.
 */
function islandora_book_ingest_page(FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/ingest.form');
  return drupal_get_form('islandora_ingest_form', array(
      'book' => $object->id,
      'namespace'=> islandora_get_namespace($object->id),
      'models' => array('islandora:pageCModel'),
    )
  );
}

/**
 * Local menu task, displays all the book management forms in a set of vertical
 * tabs.
 *
 * @param FedoraObject $object
 *   The book object to manage.
 *
 * @return array
 *   A renderable array containing all the management forms related to
 *   book objects.
 */
function islandora_book_manage_book_menu(FedoraObject $object) {
  return array(
    'manage_book' => array(
      '#type' => 'vertical_tabs',
      '#weight' => 0,
      '#prefix' => '',
      'create_pdf' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Create PDF'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_create_pdf_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      ),
      'create_ocr' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Perform OCR'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_create_ocr_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'create_images' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Create Images'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_create_images_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'reorder_pages' => array(
        // @todo Replace with check for permission.
        '#access' => TRUE,
        '#title' => t('Reorder Pages'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_reorder_pages_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      ),
      'delete_pages' => array(
        // @todo Replace with check for permission.
        '#access' => user_access(PERM_ISLANDORA_BOOK_DELETE_PAGES),
        '#title' => t('Delete Pages'),
        '#type' => 'fieldset',
        'form' => drupal_get_form('islandora_book_delete_pages_form', $object),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      )
    )
  );
}

/**
 * Creates a PDF of the book, by generating a PDF file for each page and then
 * combining them into a single PDF.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_create_pdf_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive_pdf = islandora_book_can_derive($object, 'PDF');
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>ImageMagick</b> and <b>GhostScript</b> installed to create/combine PDF files.<br/> This will also update the PDF datastreams for each Page object.'),
    ),
    'dpi' => array(
      '#type' => 'select',
      '#title' => t('DPI - Dots Per Inch'),
      '#description' => t('Set the DPI for the generated PDF. The higher the resolution the longer the process will take.'),
      '#options' => drupal_map_assoc(array('72','96','300', '600')),
    ),
    'submit' => array(
      '#disabled' => !$can_derive_pdf,
      '#type' => 'submit',
      '#value' => t('Create PDF'),
    )
  );
}

/**
 * Triggers a batch process for deriving the PDF datastreams in the books and
 * its pages.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_create_pdf_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/book.batch');
  $object = $form_state['object'];
  $pages = array_keys(islandora_book_get_pages($object));
  $options = array(
    '-density' => $form_state['values']['dpi'],
    '-compress' => 'LZW'
  );
  $batch = islandora_book_create_pdf_batch($object, $pages, $options);
  batch_set($batch);
}

/**
 * Derives the OCR datastreams in each page of the book.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_create_ocr_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_book_can_create_ocr();
  $can_preprocess = islandora_book_can_preprocess_ocr();
  $languages = islandora_book_get_enabled_tesseract_languages();
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>Tesseract</b> installed to perform OCR, and <b>Gimp</b> to preprocess OCR.<br/> This will update the OCR, RAW_HOCR, and ENCODED_OCR datastreams for each Page object.')
    ),
    'language' => array(
      '#title' => t('Language'),
      '#type' => 'select',
      '#description' => t('Please select the language that the page is written in.'),
      '#options' => $languages
    ),
    'preprocess' => array(
      '#disabled' => !$can_preprocess,
      '#title' => t('Preprocessing for typescripts?'),
      '#type' => 'checkbox',
      '#description' => t('Will add additional processing for typewritten text. This script grayscales and sharpens the uploaded ingested TIFF before performing OCR processes.<br/>This has been shown to improve OCR results for some images. Use with caution. Process can be lengthly and is resource intensive.<br/>'),
      '#default_value' => $can_preprocess
    ),
    'submit' => array(
      '#disabled' => !$can_derive,
      '#type' => 'submit',
      '#value' => t('Create OCR')
    )
  );
}

/**
 * Triggers a batch process for deriving OCR datastreams in the books pages.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_create_ocr_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/book.batch');
  $object = $form_state['object'];
  $pages = array_keys(islandora_book_get_pages($object));
  $options = array(
    'language' => $form_state['values']['language'],
    'preprocess' => (bool) $form_state['values']['preprocess'],
  );
  $batch = islandora_book_create_ocr_batch($object, $pages, $options);
  batch_set($batch);
}


/**
 * Derives the image datastreams in each page of the book.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The object to fetch the pages from.
 *
 * @return array
 *   The Drupal form.
 */
function islandora_book_create_images_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
  $can_derive = islandora_book_can_create_images();
  return array(
    'description' => array(
      '#type' => 'item',
      '#description' => t('You must have the <b>Large Image Solution Pack</b> installed to perform create images.<br/> This will update the TN, JPEG, and JP2 datastreams for each Page object.')
    ),
    'submit' => array(
      '#disabled' => !$can_derive,
      '#type' => 'submit',
      '#value' => t('Create Images')
    )
  );
}

/**
 * Triggers a batch process for deriving iamge datastreams in the books pages.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_book_create_images_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/book.batch');
  $object = $form_state['object'];
  $pages = array_keys(islandora_book_get_pages($object));
  $batch = islandora_book_create_images_batch($object, $pages);
  batch_set($batch);
}

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
 *   The Drupal form.
 */
function islandora_book_update_pages_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $default_value = function($field, $default) use($form_state) {
    return isset($form_state['values'][$field]) ? $form_state['values'][$field] : $default;
  };
  $form_state['object'] = $object;
  $languages = array_filter(variable_get('islandora_book_ocr_languages'));
  $languages = array_intersect_key(islandora_book_get_tesseract_languages(), $languages);
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
function islandora_book_update_pages_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_book', 'includes/book.batch.inc');
  $values = $form_state['values'];
  $object = $form_state['object'];
  $pages = array_values(islandora_book_get_pages($object));
  $batch = islandora_book_update_book_batch($object, $pages);
  batch_set($batch);
}


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
 *   The Drupal form.
 */
function islandora_book_reorder_pages_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora_book', 'includes/utilities');
  $form_state['object'] = $object;
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
  module_load_include('inc', 'islandora_book', 'includes/book.batch');
  $object = $form_state['object'];
  $pages = &$form_state['values']['table']['rows'];
  uasort($pages, 'drupal_sort_weight');
  $pages = array_keys($pages);
  $batch = islandora_book_reorder_pages_batch($object, $pages);
  batch_set($batch);
}

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
 *   The Drupal form.
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
  module_load_include('inc', 'islandora_book', 'includes/book.batch');
  $object = $form_state['object'];
  $pages = array_values(array_filter($form_state['values']['table']));
  $batch = islandora_book_delete_pages_batch($object, $pages);
  batch_set($batch);
}
