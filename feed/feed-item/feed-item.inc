<?php 

/**
 * Theming function; Creates a feed item table.
 */
function theme_feed_item_select($variables) {
  $elements = $variables['elements'];
  $elements += array(
    '#columns' => array('title', 'description', 'group', 'published'),
  );
  
  $operation_header = FALSE;
  $operation_colspan = 0;
  
  $rows = array();
  $header = array();
  $header[] = array('data' => '<input type="checkbox" class="feed-item-select-check-all" />', 'class' => array('feed-col-1'));
  
  $header_col = 2;
  foreach ($elements['#columns'] as $column) {
    $header_cell = array();
    
    switch ($column) {
      case 'title':
        $header_cell['data'] = !empty($elements['header']['title']) ? $elements['header']['title'] : t('Title');
      break;
      
      case 'description':
        $header_cell['data'] = !empty($elements['header']['description']) ? $elements['header']['description'] : t('Description');
      break;
      
      case 'group':
        $header_cell['data'] = !empty($elements['header']['group']) ? $elements['header']['group'] : t('Group');
      break;
      
      case 'published':
        $header_cell['data'] = !empty($elements['header']['published']) ? $elements['header']['published'] : t('Published');
      break;
      
      default: continue; break;
    }
    $header_cell['class'] = array('feed-col-' . $header_col);
    $header[] = $header_cell;
    $header_col++;
  }
  
  foreach (element_children($elements['feed_items']) as $name) {
    $element = $elements['feed_items'][$name];
    
    $row = array();
    $row[] = array('data' => array($element['checkbox']), 'class' => array('feed-col-1'));
    
    $row_col = 2;
    foreach ($elements['#columns'] as $column) {
      $row_cell = array();
      
      switch ($column) {
        case 'title':
          $row_cell['data'] = array($element['title']);
        break;
        
        case 'description':
          $row_cell['data'] = array($element['description']);
        break;
        
        case 'group':
          $row_cell['data'] = array($element['group']);
        break;
        
        case 'published':
          $row_cell['data'] = array($element['published']);
        break;
        
        default: continue; break;
      }
      $row_cell['class'] = array('feed-col-' . $row_col);
      $row[] = $row_cell;
      $row_col++;
    }
    
    if (!empty($element['operations']) && is_array($element['operations'])) {
      foreach (element_children($element['operations']) as $operation_name) {
        $operation = $element['operations'][$operation_name];
        $row[] = array('data' => array($operation), 'class' => array('feed-col-' . $row_col));
        $row_col++;
      }
      
      if (!$operation_header) {
        $operation_header = TRUE;
        $operation_colspan = count(element_children($element['operations']));
        $header[] = array(
          'data' => '&nbsp;',
          'colspan' => $operation_colspan,
          'class' => array('feed-col-' . $header_col)
        );
      }
    }
    
    $classes = array();
    if (!empty($element['#attributes']['class'])) {
      $classes += $element['#attributes']['class'];
    }
    $rows[] = array('data' => $row, 'class' => $classes);
  }
  
  if (!empty($elements['pagination'])) {
    $pages = element_children($elements['pagination']);
    if (!empty($pages)) {
      $pagination = array(
        'data' => $elements['pagination'],
        'colspan' => 4 + $operation_colspan,
      );
      $rows[] = array($pagination);
    }
  }
  
  if (!isset($elements['#attributes']['id'])) {
    $elements['#attributes']['id'] = $elements['#id'];
  }
  if (!isset($elements['#attributes']['class'])) {
    $elements['#attributes']['class'] = array();
  }
  $elements['#attributes']['class'][] = 'feed-item-select-container';
  
  return theme('feed_block', array(
    'attributes' => $elements['#attributes'],
    'title' => isset($elements['#title']) ? $elements['#title'] : 'Select Feed Items',
    'content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => isset($elements['#empty']) ? $elements['#empty'] : NULL,
      '#attributes' => array(
      	'id' => 'feed-item-select',
      ),
    ),
  ));
}

/**
 * Form process feed item select elements.
 */
function form_process_feed_item_select(&$element, &$form_state) {
  $path = $_GET['q'];
  $query = drupal_get_query_parameters();
    
  $element += array(
    '#sort' => NULL,
    '#pager' => new FeedItemPager(),
    '#page' => NULL,
    '#empty' => 'No feeds items to list',
    '#title' => 'Select Feed Items',
    '#links' => array(
      'edit' => array(
        '#title' => t('Edit'),
        '#href' => 'feed/%fid%/feed-items/%fiid%',
        '#options' => array('attributes' => array('title' => t('Edit feed item'))),
      ),
    ),
  );
  
  if (!empty($element['#page'])) {
    $element['#pager']->setCurrentPage($element['#page']);
  }
  elseif (!empty($query['feed-item-page']) && $query['feed-item-page'] > 0) {
    $element['#pager']->setCurrentPage($query['feed-item-page']);
  }
  
  if (!empty($element['#sort'])) {
    $sort = $element['#sort'];
  }
  elseif (!empty($query['feed-item-sort'])) {
    $sort = (string) $query['feed-item-page'];
  }
  
  if (!$element['#pager']->hasTag('field_title')) {
    $element['#pager']->addTag('field_title')->addFieldValueColumnByTagname('title', 'text', 'title');
  }
  if (!$element['#pager']->hasTag('field_link')) {
    $element['#pager']->addTag('field_link')->addFieldValueColumnByTagname('link', 'url', 'link');
  }
  if (!$element['#pager']->hasTag('field_description')) {
    $element['#pager']->addTag('field_description')->addFieldValueColumnByTagname('description', 'text', 'description');
  }
  if (!$element['#pager']->hasTag('field_publish')) {
    $element['#pager']->addTag('field_publish')->addFieldValueColumnByTagname('pubDate', 'timestamp', 'publish');
  }
  if (!$element['#pager']->hasTag('field_group_name') && $element['#pager']->getFeedGroupQuery()) {
    $element['#pager']->addTag('field_group_name')->getFeedItemQuery()->addField('fg', 'name', 'group_name');
  }
  
  if (!empty($sort)) {
    switch ($sort) {
      case 'title_asc':
        $element['#pager']->getFeedItemQuery()->orderBy('title', 'ASC');
      break;
      case 'title_desc':
        $element['#pager']->getFeedItemQuery()->orderBy('title', 'DESC');
      break;
      
      case 'description_asc':
        $element['#pager']->getFeedItemQuery()->orderBy('description', 'ASC');
      break;
      case 'description_desc':
        $element['#pager']->getFeedItemQuery()->orderBy('description', 'DESC');
      break;
      
      case 'published_asc':
        $element['#pager']->getFeedItemQuery()->orderBy('publish', 'ASC');
      break;
      case 'published_desc':
        $element['#pager']->getFeedItemQuery()->orderBy('publish', 'DESC');
      break;
      
      case 'group_asc':
        $element['#pager']->getFeedItemQuery()->orderBy('fg.name', 'ASC');
      break;
      case 'group_desc':
        $element['#pager']->getFeedItemQuery()->orderBy('fg.name', 'DESC');
      break;
    }
  }
  
  $element['feed_items'] = array('#tree' => TRUE);
  $element['#feed_items'] = $element['#pager']->query();
  
  foreach ($element['#feed_items'] as $feed_item) {
    $element['feed_items'][$feed_item->fiid] = array(
      '#tree' => TRUE, 
      '#attributes' => array(
        'class' => array(!empty($feed_item->publish) ? 'published' : 'unpublished'),
      ),
    );
    $element['feed_items'][$feed_item->fiid]['checkbox'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add feed item'),
      '#title_display' => 'invisible',
    );
    $element['feed_items'][$feed_item->fiid]['title'] = array(
      '#type' => 'link',
      '#attributes' => array(
        'class' => array('feed-item-select-title', 'feed-item-detailed'),
        'data-feed-item' => $feed_item->fiid,
      ),
      '#title' => feed_trim($feed_item->title, 40),
      '#href' => "feed-item/{$feed_item->fiid}/format/web",
    );
    $element['feed_items'][$feed_item->fiid]['description'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="feed-item-select-description">' . feed_trim($feed_item->description, 120) . '</div>',
    );
    $element['feed_items'][$feed_item->fiid]['group'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="feed-item-select-group">' . (!empty($feed_item->group_name) ? $feed_item->group_name : 'None') . '</div>',
    );
    $element['feed_items'][$feed_item->fiid]['published'] = array(
      '#type' => 'markup',
      '#markup' => !empty($feed_item->publish) ? date('n/j/Y', $feed_item->publish) : 'Unpublished',
    );
    
    if (!empty($element['#links']) && is_array($element['#links'])) {
      $element['feed_items'][$feed_item->fiid]['operations'] = array('#tree' => TRUE);
      foreach ($element['#links'] as $link => $option) {
        $element['feed_items'][$feed_item->fiid]['operations'][$link] = array(
          '#type' => 'link',
          '#title' => isset($option['#title']) ? $option['#title'] : $link,
          '#options' => isset($option['#options']) ? $option['#options'] : array(),
          '#href' => isset($option['#href']) ? strtr($option['#href'], array(
            '%fid%' => $feed_item->fid,
            '%fiid%' => $feed_item->fiid,
            '%title%' => $feed_item->title,
          )) : '',
        );
      }
    }
  }
	
	$title = l(t('Title'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'title_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	$description = l(t('Description'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'description_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	$group = l(t('Group'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'group_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	$published = l(t('Published'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'published_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));

	if (!empty($sort)) {
		if ($sort == 'title_asc') {
			$title = l(t('Title'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'title_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	  }
		elseif ($sort == 'title_desc') {
			$title = l(t('Title'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'title_asc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	  }
		elseif ($sort == 'description_asc') {
			$description = l(t('Description'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'description_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	  }
		elseif ($sort == 'description_desc') {
			$description = l(t('Description'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'description_asc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	  }
		elseif ($sort == 'group_asc') {
			$group = l(t('Group'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'group_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	  }
		elseif ($sort == 'group_desc') {
			$group = l(t('Group'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'group_asc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
	  }
		elseif ($sort == 'published_asc') {
			$published = l(t('Published'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'published_desc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
		}
		elseif ($sort == 'published_desc') {
			$published = l(t('Published'), $path, array('query' => array_merge($query, array('feed-item-sort' => 'published_asc')), 'attributes' => array('class' => array('feed-item-select-sort', 'feed-item-select-sort-desc'))));
		}
	}
	
	$element['header'] = array('#tree' => TRUE);
	$element['header']['title'] = array(
		'#type' => 'markup',
		'#markup' => $title,
	);
	$element['header']['description'] = array(
		'#type' => 'markup',
		'#markup' => $description,
	);
	$element['header']['group'] = array(
		'#type' => 'markup',
		'#markup' => $group,
	);
	$element['header']['published'] = array(
		'#type' => 'markup',
		'#markup' => $published,
	);
	
	if (($max_page = $element['#pager']->getMaxPage()) > 0) {
		$current_page = $element['#pager']->getCurrentPage();
		$start_page = $current_page > 5 ? $current_page - 5 : 1;
		$end_page = $current_page < $max_page - 5 ? $current_page + 5 : $max_page;
		
		$links = array();
		if ($end_page > $start_page) {
			for ($page = $start_page; $page <= $end_page; $page++) {
				$links[$page] = url($path, array('query' => array_merge($query, array('feed-item-page' => $page))));
			}
		}
		
		$element['pagination'] = array('#tree' => TRUE);
		if ($current_page > 1 && isset($links[$current_page - 1])) {
			$element['pagination']['previous'] = array(
				'#type' => 'markup',
				'#markup' => '<a href="' . $links[$current_page - 1] . '" class="feed-item-select-page">Previous</a>',
			);
		}
		
		if ($start_page > 1 && isset($links[1])) {
			$element['pagination']['first'] = array(
				'#type' => 'markup',
				'#markup' => '<a href="' . $links[1] . '" class="feed-item-select-page">...</a>',
			);
		}
		
		foreach ($links as $page => $link) {
			if ($current_page == $page) {
				$element['pagination'][$page] = array(
					'#type' => 'markup',
					'#markup' => '<span class="feed-item-select-page">' . check_plain($page) . '</span>',
				);
			}
			else {
				$element['pagination'][$page] = array(
					'#type' => 'markup',
					'#markup' => '<a href="' . $link . '" class="feed-item-select-page">' . check_plain($page) . '</a>',
				);
			}
		}
		
		if ($end_page < $max_page && isset($links[$max_page])) {
			$element['pagination']['last'] = array(
				'#type' => 'markup',
				'#markup' => '<a href="' . $links[$max_page] . '" class="feed-item-select-page">...</a>',
			);
		}
		
		if ($current_page < $max_page && isset($links[$current_page + 1])) {
			$element['pagination']['next'] = array(
				'#type' => 'markup',
				'#markup' => '<a href="' . $links[$current_page + 1] . '" class="feed-item-select-page">Next</a>',
			);
		}
	}
  return $element;
}

/**
 * Validate handler for the feed item select elements
 */
function form_validate_feed_item_select(&$element, &$form_state) {
  $feed_items = &$element['#feed_items'];
  $values = $element['#value'];
  
  $element['#feed_item_selected'] = array();
  if (!empty($values['feed_items']) && is_array($values['feed_items'])) {
    foreach ($values['feed_items'] as $fiid => $value) {
      if (!empty($value['checkbox']) && isset($feed_items[$fiid])) {
        $element['#feed_item_selected'][$feed_items[$fiid]->fiid] = &$feed_items[$fiid];
      }
    }
  }
  return $element;
}

/**
 * Form process feed item filter elements.
 */
function form_process_feed_item_filter(&$element, &$form_state, &$form) {
  drupal_add_js(drupal_get_path('module', 'feed') . '/js/feed.filter.js', 'file');
  
  $element['filter'] = array(
    '#type' => 'feed_fieldset',
    '#id' => 'feed-item-filter',
    '#title' => 'Filters',
    '#tree' => TRUE,
  );
  $element['filter']['keywords'] = array(
    '#type' => 'feed_fieldset',
    '#title' => 'Feed Attributes',
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('feed-item-filter-attributes'),
    ),
  );
  $element['filter']['keywords']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#attributes' => array(
      'class' => array('feed-filter-title', 'feed-filter-ajax-trigger'),
    ),
  );
  $element['filter']['keywords']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#attributes' => array(
      'class' => array('feed-filter-description', 'feed-filter-ajax-trigger'),
    ),
  );
  $element['filter']['advanced'] = array(
    '#type' => 'feed_fieldset',
    '#title' => 'Advanced (select all that apply)',
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('feed-filter-advanced', 'feed-filter-ajax-trigger'),
    ),
  );
  
  $element['filter']['advanced']['published'] = array(
    '#type' => 'checkbox',
    '#title' => t('Filter only published'),
    '#attributes' => array(
      'class' => array('feed-filter-published', 'feed-filter-ajax-trigger'),
    ),
  );
  $element['filter']['advanced']['terms'] = array(
    '#type' => 'feed_fieldset', 
    '#title' => t('Categories'),
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('feed-filter-category', 'feed-fieldset-collapsible'),
    ),
  );
  foreach (feed_category_load_all() as $feed_category) {
    $element['filter']['advanced']['terms'][$feed_category->fcid] = array(
      '#type' => 'checkboxes',
      '#title' => '<span class="feed-category-toggle">+</span>' . $feed_category->name,
      '#options' => feed_category_term_options($feed_category),
      '#attributes' => array(
        'class' => array('feed-filter-category-terms', 'feed-filter-ajax-trigger'),
      ),
    );
  }
  
  $element['filter']['advanced']['group'] = array(
    '#type' => 'feed_fieldset',
    '#title' => t('Content Source'),
    '#attributes' => array(
      'class' => array('feed-filter-group', 'feed-fieldset-collapsible'),
    ),
  );
  foreach (feed_group_load_all() as $feed_group) {
    $element['filter']['advanced']['group'][$feed_group->fgid] = array(
      '#type' => 'checkbox',
      '#title' => $feed_group->name,
      '#default_value' => FALSE,
      '#attributes' => array(
        'class' => array('feed-filter-ajax-trigger')
      ),
    );
  }
  
  $element['filter']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#ajax' => $element['#ajax'],
    '#attributes' => array(
      'class' => array('feed-filter-submit', 'feed-filter-ajax-submit'),
    ),
  );
  return $element;
}

/**
 * Form after build handler for the feed item filter elements
 */
function form_after_build_feed_item_filter(&$element, &$form_state) {
  if (empty($element['#pager']) || !$element['#pager'] instanceof FeedItemPager) {
    $element['#pager'] = new FeedItemPager(20);
  }
  
  $values = $element['#value'];
  
  if (!empty($values)) {
    if (!empty($values['advanced']['published'])) {
      $element['#pager']->hasFieldValueByTagname('pubDate', array('timestamp' => '0'), '>');
    }
    
    $fgids = array();
    if (!empty($values['advanced']['group'])) {
      foreach ($values['advanced']['group'] as $fgid => $checked) {
        if (!empty($checked) && $feed_group = feed_group_load($fgid)) {
          $fgids[] = $feed_group->fgid;
        }
      }
    }
    
    if (!empty($fgids)) {
      $element['#pager']->hasFeedGroupId($fgids);
    }
    
    foreach ($values['keywords'] as $name => $keyword) {
      if (!empty($keyword)) {
        switch ($name) {
          case 'title':
            $element['#pager']->hasFieldValueByTagname('title', array('text' => '%' . db_like($keyword) . '%'), 'LIKE');
          break;
          
          case 'description':
            $element['#pager']->hasFieldValueByTagname('description', array('html' => '%' . db_like($keyword) . '%'), 'LIKE');
          break;
        }
      }
    }
    
    if (!empty($values['advanced']['terms'])) {
      foreach ($values['advanced']['terms'] as $fcid => $terms) {
        $selected = array();
        
        foreach ($terms as $fctid => $checked) {
          if (!empty($checked) && $feed_category_term = feed_category_term_load($fctid)) {
            $selected[$feed_category_term->fctid] = $feed_category_term->fctid;
          }
        }
        
        if (!empty($selected)) {
          $element['#pager']->hasFeedItemTagIds($selected);
        }
      }
    }
  }
  return $element;
}

/**
 * Return an array of subscribed feed item objects.
 * 
 * @param $feed
 *   A feed object.
 *
 * @return
 *   An array of all feed item objects, indexed by fiid.
 */
function feed_load_subscribed_feed_items($feed) {
  $fiids = feed_load_subscribed_fiids($feed);
  
  return !empty($fiids) ? feed_items_load($fiids) : array();
}

/**
 * Return an array of subscribed fiids.
 * 
 * @param $feed
 *   A feed object.
 *
 * @return
 *   An array of fiids.
 */
function feed_load_subscribed_fiids($feed) {
  $select = db_select('feed_item', 'fi');
  $select->join('feed_item_subscription', 'fis', 'fis.fiid = fi.fiid');
  $select->fields('fi', array('fiid'));
  $select->condition('fis.fid', $feed->fid);
  $select->orderBy('fi.fiid', 'DESC');
  return $select->execute()->fetchAllKeyed(0, 0);
}

/**
 * Return an array of feed item objects.
 * 
 * @param $feed
 *   A feed object.
 * @param $offset
 *   Starting offset to retrieve feed items of this feed.
 * @param $limit
 *   The total number of feed items to retrieve.
 *
 * @return
 *   An array of all feed item objects, indexed by fiid.
 */
function feed_load_created_feed_items($feed) {
  $select = db_select('feed_item', 'fi');
  $select->fields('fi', array('fiid'));
  $select->condition('fi.fid', $feed->fid);
  $select->orderBy('fi.fiid', 'DESC');
  $fiids = $select->execute()->fetchAllKeyed(0, 0);
  
  return feed_items_load($fiids);
}

/**
 * Load a feed item by field value
 *
 * @param $feed_item_field
 *   The feed item field object
 * @param $name
 *   The feed item field value name
 * @param $value
 *   The feed item field value data
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed item.
 */
function feed_item_load_from_field_value($feed_item_field, $name, $data, $reload = FALSE) {
  $query = db_select('feed_item_field_value_data', 'fifvd');
  $query->join('feed_item_field_value', 'fifv', 'fifv.fifvid = fifvd.fifvid');
  $query->distinct(TRUE);
  $query->fields('fifv', array('fiid'));
  $query->condition('fifv.ffid', $feed_item_field->ffid);
  $query->condition('fifvd.name', $name);
  $query->condition('fifvd.data', $data);
  
  $fiids = $query->execute()->fetchAllKeyed(0, 0);
  
  return !empty($fiids) ? feed_items_load($fiids) : array();
}

/**
 * Load a feed item.
 *
 * @param $fiid
 *   The feed item id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed item.
 */
function feed_item_load($fiid, $reload = FALSE) {
  return end(feed_items_load(array($fiid), $reload));
}


/**
 * Load feed items.
 *
 * @param $fiids
 *   An array of feed item id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of object describing the feed item.
 */
function feed_items_load(array $fiids, $reload = FALSE) {
  $feed_items = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_items)) {
    $feed_items = array();
  }
  $diff_fiids = array_diff($fiids, array_keys($feed_items));
  if ($reload || !empty($diff_fiids)) {
    $diff_feed_items = db_select('feed_item', 'fi')
      ->fields('fi')
      ->condition('fi.fiid', $diff_fiids)
      ->execute()->fetchAllAssoc('fiid');
      
    drupal_alter('feed_items_load', $diff_feed_items);
      
    foreach ($diff_feed_items as $diff_feed_item) {
      $feed_items[$diff_feed_item->fiid] = $diff_feed_item;
    }
  }
  
  $results = array();
  foreach ($fiids as $fiid) {
    if (isset($feed_items[$fiid])) {
      $results[$feed_items[$fiid]->fiid] = $feed_items[$fiid];
    }
  }
  return $results;
}

/**
 * Save a feed item.
 *
 * @param $feed item
 *   The feed item to save.
 * @param $feed
 *   The feed to save feed item to.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_item_save(&$feed_item, $feed, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $feed_item->fid = $feed->fid;
    $feed_item->updated_at = REQUEST_TIME;
    if (!isset($feed_item->created_at)) {
      $feed_item->created_at = REQUEST_TIME;
    }
    drupal_alter('feed_item_save', $feed_item);
  
    if (!$status = drupal_write_record('feed_item', $feed_item, isset($feed_item->fiid) ? 'fiid' : array())) {
      throw new Exception('Could not save a feed item record');
        
      // @todo Other stuff
    }
    db_ignore_slave();
    
    return $status;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Updates the update timestamp column for an array of feed item objects
 * 
 * @param $feed_items
 *   An array of feed item objects.
 */
function feed_items_update_timestamp(array &$feed_items) {
  $fiids = array();
  
  foreach ($feed_items as &$feed_item) {
    $fiids[] = $feed_item->fiid;
  }
  
  return db_update('feed_item')
    ->fields(array('updated_at' => REQUEST_TIME))
    ->condition('fiid', $fiids, 'IN')
    ->execute();
}

/**
 * Deletes a feed item from the database.
 *
 * @param $feed_item
 *   The feed item to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_item_delete($feed_item, $transaction = NULL) {
  return feed_items_delete(array($feed_item), $transaction);
}

/**
 * Delete feed items.
 *
 * @param $fiids
 *   An array of feed item's fid to delete.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_items_delete(array $fiids, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    foreach ($fiids as &$fiid) {
      if (is_object($fiid) && isset($fiid->fiid)) {
        $fiid = $fiid->fiid;
      }
        
      $fifvids = db_select('feed_item_field_value', 'fifv')
        ->fields('fifv', array('fifvid'))
        ->condition('fiid', $fiid)
        ->execute()->fetchAllKeyed(0, 0);
        
      if (!empty($fifvids)) {
        if (!feed_item_field_values_delete($fifvids, $transaction)) {
          throw new Exception('Unable to delete feed item field values');
        }
      }
    }
    
    db_delete('feed_item')
      ->condition('fiid', $fiids)
      ->execute();
    
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Publish feed items.
 * 
 * @param $feed_items
 *   Array of feed item objects.
 */
function feed_item_publish(array &$feed_items) {
  $success = array();
  foreach ($feed_items as &$feed_item) {
    if (isset($feed_item->fiid)) {
      $feed_item_fields = feed_item_field_tagname_load('pubDate', $feed_item->fid);
      
      foreach ($feed_item_fields as $feed_item_field) {
        $feed_item_field_values = feed_item_field_values_load($feed_item, $feed_item_field);
        
        if (!empty($feed_item_field_values)) {
          foreach ($feed_item_field_values as &$feed_item_field_value) {
            $feed_item_field_value->data[FEED_DATA_TYPE_FIELD_VALUE]['timestamp'] = REQUEST_TIME;
          }
        }
        else {
          $feed_item_field_values['new'] = feed_item_field_value_defaults();
          $feed_item_field_values['new']->data[FEED_DATA_TYPE_FIELD_VALUE]['timestamp'] = REQUEST_TIME;
        }
        $status = feed_item_field_values_save($feed_item_field_values, $feed_item, $feed_item_field);
        
        if (!isset($success[$feed_item->fiid]) || !empty($success[$feed_item->fiid])) {
          $success[$feed_item->fiid] = $status;
        }
      }
    }
  }
  return $success;
}

/**
 * Unpublish feed items.
 * 
 * @param $feed_items
 *   Array of feed item objects.
 */
function feed_item_unpublish(array &$feed_items) {
  $success = array();
  foreach ($feed_items as &$feed_item) {
    if (isset($feed_item->fiid)) {
      $feed_item_fields = feed_item_field_tagname_load('pubDate', $feed_item->fid);
      foreach ($feed_item_fields as $feed_item_field) {
        $feed_item_field_values = feed_item_field_values_load($feed_item, $feed_item_field);
        
        $status = feed_item_field_values_delete($feed_item_field_values);
        
        if (!isset($success[$feed_item->fiid]) || !empty($success[$feed_item->fiid])) {
          $success[$feed_item->fiid] = $status;
        }
      }
    }
  }
  return $success;
}

/**
 * Configure feed item object/ensure defaults
 * 
 * @param $feed_item
 * 	 A feed item object.
 */
function feed_item_defaults($feed_item = NULL) {
  $feed_item = (object) $feed_item;
  
  return $feed_item;
}