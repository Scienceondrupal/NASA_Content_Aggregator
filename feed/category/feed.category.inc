<?php

/**
 * Theming function; Creates a feed category terms table.
 */
function theme_feed_category_term($variables) {
  $elements = $variables['elements'];
  
  // Headers.
  $header = array(
    array('data' => 'Name'),
    array('data' => 'Weight'),
    array('data' => 'Operations', 'colspan' => '2'),
  );
  
  $rows = array();
  
  // Field Rows. 
  if (!empty($elements['terms'])) {
    foreach (element_children($elements['terms'], TRUE) as $name) {
      $field = $elements['terms'][$name];
      
      $row = array();
      $row[] = array('data' => array($field['name']));
      $row[] = array('data' => array($field['weight'], $field['id']));
      $row[] = isset($field['edit']) ? array('data' => array($field['edit'])) : array('data' => '&nbsp;');
      $row[] = isset($field['edit']) ? array('data' => array($field['delete'])) : array('data' => '&nbsp;');
      
      $rows[] = array(
        'class' => array('draggable'),
        'data' => $row,
      );
    }
  }
  
  // New field Rows.
  $row = array();
  $row[] = array('data' => array($elements['add-term']['name'], $elements['add-term']['add']));
  $row[] = array('data' => $elements['add-term']['weight']);
  $row[] = array('data' => '&nbsp;');
  $row[] = array('data' => '&nbsp;');
  
  $rows[] = array('data' => $row, 'class' => array('draggable'));
  
  drupal_add_tabledrag('feed-category-term', 'order', 'sibling', 'feed-category-term-weight');
  
  return theme('feed_block', array(
    'attributes' => array('id' => 'feed-category-terms-overview'),
    'title' => 'Feed Category Terms',
    'content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    	'#empty' => t('No terms in this category.'),
      '#attributes' => array(
      	'id' => 'feed-category-term',
      ),
    ),
  ));
}

/**
 * Load a feed category.
 *
 * @param $fcid
 *   The feed category id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed category.
 */
function feed_category_load($fcid, $reload = FALSE) {
  $feed_categories = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_categories)) {
    $feed_categories = array();
  }
  if ($reload || !isset($feed_categories[$fcid])) {
    $feed_categories[$fcid] = db_query('SELECT * FROM {feed_category} WHERE fcid = :fcid', array(':fcid' => $fcid))->fetchObject();
  }
  return $feed_categories[$fcid];
}

/**
 * Return an array of all feed category objects belonging to user
 * 
 * @param $uid
 *   The User id to load feed category objects for.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed category objects, indexed by fcid.
 */
function feed_category_load_all_user($uid = NULL, $reload = FALSE) {
  global $user;
  
  if (empty($uid)) {
    $uid = $user->uid;
  }
  $feed_user_categories = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_user_categories)) {
    $feed_user_categories = db_select('feed_category', 'fc')
      ->fields('fc')
      ->condition(db_or()
        ->condition('fc.uid', $uid)
        ->condition('fc.is_public', TRUE))
      ->execute()->fetchAllAssoc('fcid');
  }
  return $feed_user_categories;
}

/**
 * Return an array of all feed category objects.
 *
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed category objects, indexed by fcid.
 */
function feed_category_load_all($reload = FALSE) {
  $feed_categories = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_categories)) {
    $feed_categories = db_query('SELECT * FROM {feed_category} WHERE 1 = 1')->fetchAllAssoc('fcid');
  }
  return $feed_categories;
}

/**
 * Save a feed category.
 *
 * @param $feed_category
 *   The feed category to save.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_category_save(&$feed_category, DatabaseTransaction $transaction = NULL) {
  global $user;
  
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  try {
	  if (empty($feed_category->uid)) {
	    $feed_category->uid = $user->uid;
	  }
    if (!$status = drupal_write_record('feed_category', $feed_category, isset($feed_category->fcid) ? 'fcid' : array())) {
      throw new Exception('Could not save feed category record');
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
 * Deletes a feed category from the database.
 *
 * @param $feed_category
 *   The feed category to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_category_delete($feed_category, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_category->fcid)) {
      $query = db_select('feed_category_term_index', 'fcti');
      $query->join('feed_category_term', 'fct', 'fcti.fctid = fct.fctid');
      $query->fields('fcti', array('fctid'));
      $query->condition('fct.fcid', $feed_category->fcid);
      
      $fctids = $query->execute()->fetchAllKeyed(0, 0);
        
      if (!empty($fctids)) {
        db_delete('feed_category_term_index')->condition('fcid', $fctids)->execute();
      }
      
      $item_query = db_select('feed_item_category_term_index', 'ficti');
      $item_query->join('feed_category_term', 'fct', 'ficti.fctid = fct.fctid');
      $item_query->fields('ficti', array('fctid'));
      $item_query->condition('fct.fcid', $feed_category->fcid);
      
      $fictids = $item_query->execute()->fetchAllKeyed(0, 0);
        
      if (!empty($fctids)) {
        db_delete('feed_category_term_index')->condition('fcid', $fictids)->execute();
      }
      
      db_delete('feed_category')
        ->condition('fcid', $feed_category->fcid)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Return an array of feed category as options
 * 
 * @return 
 *   A key/value array of feed category useful for select option.
 */
function feed_category_options() {
  $feed_category_options = &drupal_static(__FUNCTION__, array());
  
  if (empty($feed_category_options)) {
    if ($feed_categories = feed_category_load_all()) {
      foreach ($feed_categories as $fcid => $feed_category) {
        $feed_category_options[$feed_category->fcid] = $feed_category->name;
      }
    }
  }
  return $feed_category_options;
}

/**
 * Configure feed category object/ensure defaults
 * 
 * @param $feed_category
 * 	 A feed category object.
 */
function feed_category_defaults($feed_category = NULL) {
  $feed_category = (object) $feed_category;
  
  if (!isset($feed_category->name)) $feed_category->name = '';
  if (!isset($feed_category->description)) $feed->data = '';
  if (!isset($feed_category->is_public)) $feed_category->is_public = TRUE;
  
  return $feed_category;
}

/**
 * Load a feed category term.
 *
 * @param $fctid
 *   The feed category term id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed category term.
 */
function feed_category_term_load($fctid, $reload = FALSE) {
  return end(feed_category_terms_load(array($fctid), $reload));
}

/**
 * Load a feed category term by term
 *
 * @param $term
 *   The feed category term .
 * @param $feed_category
 *   The feed category object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed category term.
 */
function feed_category_term_load_term($term, $feed_category, $reload = FALSE) {
  $feed_category_terms = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_category_terms)) {
    $feed_category_terms = array();
  }
  if (!isset($feed_category_terms[$feed_category->fcid])) {
    $feed_category_terms[$feed_category->fcid] = array();
  }
  
  if ($reload || !isset($feed_category_terms[$feed_category->fcid][$term])) {
    $feed_category_terms[$feed_category->fcid][$term] = db_select('feed_category_term', 'fct')
      ->fields('fct')
      ->condition('fct.name', $term)
      ->condition('fct.fcid', $feed_category->fcid)
      ->execute()->fetchAssoc();
  }
  return $feed_category_terms[$feed_category->fcid][$term];
}

/**
 * Load feed category terms.
 *
 * @param $fctid
 *   An array of feed category term id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of object describing the feed category term.
 */
function feed_category_terms_load(array $fctids, $reload = FALSE) {
  $feed_category_terms = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_category_terms)) {
    $feed_category_terms = array();
  }
  $diff_fctids = array_diff($fctids, array_keys($feed_category_terms));
  if ($reload || !empty($diff_fctids)) {
    $diff_feed_category_terms = db_select('feed_category_term', 'fct')
      ->fields('fct')
      ->condition('fct.fctid', $diff_fctids)
      ->execute()->fetchAllAssoc('fctid');
      
    foreach ($diff_feed_category_terms as $diff_feed_category_term) {
      $feed_category_terms[$diff_feed_category_term->fctid] = $diff_feed_category_term;
    }
  }
  
  $results = array();
  foreach ($fctids as $fctid) {
    if (isset($feed_category_terms[$fctid])) {
      $results[$feed_category_terms[$fctid]->fctid] = $feed_category_terms[$fctid];
    }
  }
  return $results;
}

/**
 * Return an array of all feed category term objects for a feed category.
 *
 * @param $feed_category
 * 	 A feed category object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed category term objects, indexed by fctid.
 */
function feed_category_term_load_all($feed_category, $reload = FALSE) {
  $feed_category_terms = drupal_static(__FUNCTION__);

  if (!isset($feed_category_terms)) {
    $feed_category_terms = array();
  }
  if ($reload || !isset($feed_category_terms[$feed_category->fcid])) {
    $feed_category_terms[$feed_category->fcid] = db_query('SELECT * FROM {feed_category_term} WHERE fcid = :fcid', array(':fcid' => $feed_category->fcid))->fetchAllAssoc('fctid');
  }
  return $feed_category_terms[$feed_category->fcid];
}

/**
 * Return an array of feed category term as option
 * 
 * @param $feed_category
 *   A feed category object.
 *   
 * @return
 *   A key/value array of feed category terms useful for select option.
 */
function feed_category_term_options($feed_category) {
  $feed_category_term_options = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_category_term_options)) {
    $feed_category_term_options = array();
  }
    
  if (!isset($feed_category_term_options[$feed_category->fcid])) {
    if ($feed_category_terms = feed_category_term_load_all($feed_category)) {
      foreach ($feed_category_terms as $fctid => $feed_category_term) {
        $feed_category_term_options[$feed_category->fcid][$feed_category_term->fctid] = $feed_category_term->name;
      }
    }
  }
  return isset($feed_category_term_options[$feed_category->fcid]) ? $feed_category_term_options[$feed_category->fcid] : array();
}

/**
 * Save a feed category term.
 *
 * @param $feed_category_term
 *   The feed category term to save.
 * @param $feed_category
 *   The feed category to save feed category term to.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_category_term_save(&$feed_category_term, $feed_category, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $feed_category_term->fcid = $feed_category->fcid;
    
    if (!$status = drupal_write_record('feed_category_term', $feed_category_term, isset($feed_category_term->fctid) ? 'fctid' : array())) {
      throw new Exception('Could not save feed category term record');
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
 * Save feed category terms.
 *
 * @param $feed_category_terms
 *   An array of feed category terms to save.
 * @param $feed_category
 *   A feed category object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *   
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_category_terms_save(array &$feed_category_terms, $feed_category, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    foreach ($feed_category_terms as &$feed_category_term) {
      $feed_category_term->fcid = $feed_category->fcid;
      
      if (!$status = drupal_write_record('feed_category_term', $feed_category_term, isset($feed_category_term->fctid) ? 'fctid' : array())) {
        throw new Exception('Could not save feed category term record');
      }
    }
    db_ignore_slave();
    
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Deletes a feed category term from the database.
 *
 * @param $feed_category_term
 *   The feed category term to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_category_term_delete($feed_category_term, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_category_term->fctid)) {
      db_delete('feed_category_term')
        ->condition('fctid', $feed_category_term->fctid)
        ->execute();
        
      db_delete('feed_category_term_index')
        ->condition('fctid', $feed_category_term->fctid)
        ->execute();
        
      db_delete('feed_item_category_term_index')
        ->condition('fctid', $feed_category_term->fctid)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Configure feed category object/ensure defaults
 * 
 * @param $feed_category_term
 * 	 A feed category term object.
 */
function feed_category_term_defaults($feed_category_term = NULL) {
  global $user;
  
  $feed_category_term = (object) $feed_category_term;
  
  if (!isset($feed_category_term->name)) $feed_category_term->name = '';
  if (!isset($feed_category_term->weight)) $feed_category_term->weight = 0;
  if (!isset($feed_category_term->uid)) $feed_category_term->uid = $user->uid;
  
  return $feed_category_term;
}

/**
 * Retrieves a list of feed category term objects a feed item is tagged with.
 * 
 * @param $feed_item
 *   A feed item object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 */
function feed_item_category_terms($feed_item, $reload = FALSE) {
  $feed_category_terms = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_category_terms)) {
    $feed_category_terms = array();
  }
  
  if ($reload || !isset($feed_category_terms[$feed_item->fiid])) {
    $fctids = db_select('feed_item_category_term_index', 'ficti')
      ->fields('ficti', array('fctid'))
      ->condition('ficti.fiid', $feed_item->fiid)
      ->execute()->fetchAllKeyed(0, 0);
      
    if (!empty($fctids)) {
      $feed_category_terms[$feed_item->fiid] = feed_category_terms_load($fctids, $reload);
    }
  }
  return isset($feed_category_terms[$feed_item->fiid]) ? $feed_category_terms[$feed_item->fiid] : array();
}

/**
 * Tags a feed item with a category term.
 * 
 * @param $feed_item
 *   A feed item object.
 * @param $feed_category_term
 *   A feed category term object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_item_tag_category_term($feed_item, $feed_category_term, DatabaseTransaction $transaction = NULL) {
  return feed_item_tag_category_terms($feed_item, array($feed_category_term), $transaction);
}

/**
 * Tags a feed item with an array of category term.
 * 
 * @param $feed_item
 *   A feed item object.
 * @param $feed_category_term
 *   An array of feed category term objects.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_item_tag_category_terms($feed_item, array $feed_category_terms, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $fctids = array();
    foreach ($feed_category_terms as $feed_category_term) {
      if (isset($feed_category_term->fctid)) {
        $fctids[$feed_category_term->fctid] = $feed_category_term->fctid;
      }
    }
    
    if (isset($feed_item->fiid)) {
      $feed_item_category_term_indexes = db_select('feed_item_category_term_index', 'ficti')
        ->fields('ficti', array('fctid'))
        ->condition('ficti.fiid', $feed_item->fiid)
        ->condition('ficti.fctid', $fctids)
        ->execute()->fetchAllKeyed(0, 0);
        
      $fctids = array_diff($fctids, $feed_item_category_term_indexes);
      
      if (!empty($fctids)) {
        $insert = db_insert('feed_item_category_term_index')->fields(array('fiid', 'fctid'));
        
        foreach ($fctids as $fctid) {
          $insert->values(array('fiid' => $feed_item->fiid, 'fctid' => $fctid));
        }
        $insert->execute();
      }
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Untag a feed item with a category term.
 * 
 * @param $feed_item
 *   A feed item object.
 * @param $feed_category_term
 *   A feed category term object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_item_untag_category_term($feed_item, $feed_category_term, DatabaseTransaction $transaction = NULL) {
  return feed_item_untag_category_terms($feed_item, array($feed_category_term), $transaction);
}

/**
 * Untags a feed item with an array of category terms.
 * 
 * @param $feed_item
 *   A feed item object.
 * @param $feed_category_terms
 *   An array of feed category term objects.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_item_untag_category_terms($feed_item, array $feed_category_terms, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $fctids = array();
    foreach ($feed_category_terms as $feed_category_term) {
      if (isset($feed_category_term->fctid)) {
        $fctids[$feed_category_term->fctid] = $feed_category_term->fctid;
      }
    }
    
    if (isset($feed_item->fiid) && !empty($fctids)) {
      db_delete('feed_item_category_term_index')
        ->condition('fiid', $feed_item->fiid)
        ->condition('fctid', $fctids)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Untags all category terms from a feed item.
 * 
 * @param $feed_item
 *   A feed item object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_item_untag_all_category_terms($feed_item, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_item->fiid)) {
      db_delete('feed_item_category_term_index')
        ->condition('fiid', $feed_item->fiid)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Tags a feed with a category term.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_category_term
 *   A feed category term object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_tag_category_term($feed, $feed_category_term, DatabaseTransaction $transaction = NULL) {
  return feed_tag_category_terms($feed, array($feed_category_term), $transaction);
}

/**
 * Tags a feed with an array of category term.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_category_term
 *   An array of feed category term objects.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_tag_category_terms($feed, array $feed_category_terms, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $fctids = array();
    foreach ($feed_category_terms as $feed_category_term) {
      if (isset($feed_category_term->fctid)) {
        $fctids[$feed_category_term->fctid] = $feed_category_term->fctid;
      }
    }
    
    if (isset($feed->fid)) {
      $feed_category_term_indexes = db_select('feed_category_term_index', 'fcti')
        ->fields('fcti', array('fctid'))
        ->condition('fcti.fid', $feed->fid)
        ->condition('fcti.fctid', $fctids)
        ->execute()->fetchAllKeyed(0, 0);
        
      $fctids = array_diff($fctids, $feed_category_term_indexes);
      
      if (!empty($fctids)) {
        $insert = db_insert('feed_category_term_index')->fields(array('fid', 'fctid'));
        
        foreach ($fctids as $fctid) {
          $insert->values(array('fid' => $feed->fid, 'fctid' => $fctid));
        }
        $insert->execute();
      }
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Untag a feed with a category term.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_category_term
 *   A feed category term object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_untag_category_term($feed, $feed_category_term, DatabaseTransaction $transaction = NULL) {
  return feed_untag_category_terms($feed, array($feed_category_term), $transaction);
}

/**
 * Untags a feed with an array of category terms.
 * 
 * @param $feed
 *   A feed object.
 * @param $feed_category_terms
 *   An array of feed category term objects.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_untag_category_terms($feed, array $feed_category_terms, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    $fctids = array();
    foreach ($feed_category_terms as $feed_category_term) {
      if (isset($feed_category_term->fctid)) {
        $fctids[$feed_category_term->fctid] = $feed_category_term->fctid;
      }
    }
    
    if (isset($feed->fid) && !empty($fctids)) {
      db_delete('feed_category_term_index')
        ->condition('fid', $feed->fid)
        ->condition('fctid', $fctids)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Untags all category terms from a feed.
 * 
 * @param $feed
 *   A feed object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_untag_all_category_terms($feed, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed->fid)) {
      db_delete('feed_category_term_index')
        ->condition('fid', $feed->fid)
        ->execute();
    }
    return TRUE;
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception('feed', $e);
    return FALSE;
  }
}

/**
 * Query function for feed category term autocompleter
 */
function feed_category_term_autocomplete($search) {
  $search = (string) $search;
  
  $select = db_select('feed_category_term', 'fct');
  $select->fields('fct', array('fctid', 'fcid', 'name'));
  $select->join('feed_category', 'fc', 'fc.fcid = fct.fcid');
  $select->addField('fc', 'name', 'category');
  $select->condition(db_or()
  	->condition('fc.name', '%' . db_like($search) . '%', 'LIKE')
    ->condition('fct.name', '%' . db_like($search) . '%', 'LIKE')
  );
  $select->range(0, 100);
  
  $results = array();
  foreach ($select->execute()->fetchAllAssoc('fctid') as $term) {
    $results[$term->fctid] = $term;
  }
  return $results;
}