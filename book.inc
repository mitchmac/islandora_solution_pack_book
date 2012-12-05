<?php

/**
 * @file
 *
 */

/**
 * Islandora Book
 */
class IslandoraBook {

  /**
   * Instantiate
   *
   * @param string $pid
   */
  function __construct(FedoraObject $object) {
    $this->object = $object;
  }

  /**
   * Show field set
   *
   * @global string $base_url
   * @global user $user
   * @return string
   */
  public function showFieldSets() {
    global $user;
    module_load_include('inc', 'islandora_book', 'book_pack_utils');
    module_load_include('inc', 'islandora_book', 'add_pages');
    module_load_include('module', 'islandora_book');
    module_load_include('inc', 'fedora_repository', 'CollectionClass');
    $path = drupal_get_path('module', 'islandora_book');
    drupal_add_css($path . '/css/islandora_book.css');

    drupal_add_js($path . '/js/book.js?stopcache');
    $show_purge_tab = (!empty($_POST['form_id']) && ($_POST['form_id'] == 'islandora_book_purge_pages_form'));
    $page_number = 1;
    $pages_selected = FALSE;
    $qstring = $_GET['q'];
    $qparts = explode('/', $qstring);
    $tail = end($qparts);
    if (is_numeric($tail)) {
      $pages_selected = TRUE;
      $page_number = $tail;
    }
    $class = new CollectionClass();
    $query = <<<XML
select \$object \$title  from <#ri>
where \$object <fedora-rels-ext:isMemberOf> <info:fedora/$this->object->id>
and \$object <fedora-model:state> <fedora-model:Active>
and (\$object <info:islandora/islandora-system:def/paged-item-info#sequenceNumber> \$title
or \$object <http://islandora.ca/ontology/relsext#isSequenceNumber> \$title)
order by \$title
XML;
    $results = $class->getRelatedItems($this->object->id, $query);
    $page_items = $class->renderCollection($results, $this->object->id, NULL, NULL, $page_number);
    $page_pids = get_page_pids($this->object->id);

    if (count($page_pids) > 0) {
      $hasPage = TRUE;
    }
    $tabset = array();

    global $user;
    $qs = '';
    if ($user->uid != 0) {
      $qs = '?uid=' . base64_encode($user->name . ':' . $user->pass);
    }
    module_load_include('inc', 'islandora_bookviewer', 'islandora_bookviewer');
    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $oh = new ObjectHelper();
    //This is a bit of a hack but allows us to handle two viewers in one solution pack
    //ideally we would do module invoke_all and then implement the viewers in a module
    //but two modules for 4 lines of code seems a bit much.
    $sequence_number = (empty($_GET['startpage'])) ? '1' : $_GET['startpage'];
    $cmodels = $oh->get_content_models_list($this->object->id);
    foreach ($cmodels as $cmodel) {
      if ($cmodel->pid == "islandora:iaBookCModel") {
        $bookviewer = new IslandoraBookviewer($this->object->id);
        $html = $bookviewer->showAjaxBookViewer('mainpage.php', $sequence_number);
      }
      else {
        $viewer_url = variable_get('fedora_base_url', '') . '/get/' . $this->object->id . '/islandora:viewerSdef/getViewer' . $qs;
        $html = '<iframe src="' . $viewer_url . '" scrolling="0" frameborder="0" style="width: 100%; height: 800px;">Errors: unable to load viewer</iframe>';
      }
    }
    $tabset['pages_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('Pages'),
      '#selected' => $pages_selected,
      '#content' => $page_items,
      '#weight' => 15,
    );

    $tabset['first_tab'] = array(
      // #type and #title are the minimum requirements.
      '#type' => 'tabpage',
      '#title' => t('About'),
      '#weight' => 1,
      // This will be the content of the tab.
      '#content' => $this->create_book_view(),
    );

    if ($hasPage) {
      $tabset['second_tab'] = array(
        '#type' => 'tabpage',
        '#title' => t('Reading Edition'),
        '#content' => $html,
        '#weight' => -10,
      );
    }

    return $tabset;
  }

  function create_book_view() {
    global $base_url;
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    module_load_include('inc', 'fedora_repository', 'ObjectDetails');
    $headers = array($item->objectProfile->objLabel, "");
    $item = new Fedora_Item($this->object->id);

    //$id = 'islandora:557';
    //echo theme_image(array('path' => "islandora/object/{$id}/datastreams/TN/view", 'attributes' => array()));

    $img = theme_image("$base_url/fedora/repository/" . $this->object->id . "/TN", '', NULL, NULL, FALSE);
    $output = "";
    if (user_access('manage book object')) {
      $links = array();
      $links[] = l(t('Edit Metadata'), "fedora/repository/editmetadata/$this->object->id/metadata");
      $links[] = l(t('Manage Book'), "islandora/book/management/$this->object->id/");
      $links = array_merge($links, module_invoke_all('islandora_book_get_bookview_links', $this->object->id));
      $output .= theme_item_list($links);
    }

    $output .= fedora_repository_object_details_XSLT($item);
    $headers = array($item->objectProfile->objLabel, "");
    $rows = array(array($img, $output));
    return theme_table($headers, $rows);
  }

}
