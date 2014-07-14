<?php 

/**
 * Load a feed from fid.
 *
 * @param $fid
 *   The feed id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *
 * @return
 *   An object describing the feed.
 */
function feed_load($fid, $reload = FALSE) {
  return end(feeds_load(array($fid), $reload));
}

/**
 * Load an array of feed from fid.
 * 
 * @param $fids
 *   An array of fids.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *
 * @return
 *   An array of object describing the feed indexed by fid.
 */
function feeds_load($fids, $reload = FALSE) {
  global $user;
  
  $feeds = &drupal_static(__FUNCTION__);
  
  if (!isset($feeds)) {
    $feeds = array();
  }
  
  if ($reload) {
    $diff_fids = $fids;
  }
  else {
    $diff_fids = array_diff($fids, array_keys($feeds));
  }
  
  if (!empty($diff_fids)) {
    $diff_feeds = db_select('feed', 'f')
      ->fields('f')
      ->condition('f.fid', $diff_fids)
      ->execute()->fetchAllAssoc('fid');
      
    foreach ($diff_feeds as $diff_feed) {
      if (!empty($diff_feed->data)) {
        $diff_feed->data = @unserialize($diff_feed->data);
      }
      $feeds[$diff_feed->fid] = $diff_feed;
    }
  }
  
  $results = array();
  foreach ($fids as $fid) {
    if (isset($feeds[$fid])) {
      $results[$feeds[$fid]->fid] = $feeds[$fid];
    }
  }
  return $results;
}

/**
 * Return an array of all feed objects.
 *
 * @param $type
 *   A feed type. If specified will only return feeds of that type, or else will
 *   return all feeds.
 * @param $user
 *   A user object. If specified will only return feeds owned by this user.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed objects, indexed by fid.
 */
function feed_load_all($type = NULL, $user = FALSE, $reload = FALSE) {
  if ($type !== NULL) {
    if (is_array($type)) {
      $list = array();
      foreach ($type as $feed_type) {
        $list = array_merge($list, feed_load_all($feed_type, $user, $reload));
      }
      return $list;
    }
    else {
      $select = db_select('feed', 'f');
      $select->fields('f', array('fid'));
      $select->condition('type', $type);
      if (isset($user->uid)) {
        $select->condition('uid', $user->uid);
      }
      $fids = $select->execute()->fetchAllKeyed(0, 0);
      
      return feeds_load($fids, $reload);
    }
  }
  else {
    $select = db_select('feed', 'f');
    $select->fields('f', array('fid'));
    if (isset($user->uid)) {
      $select->condition('uid', $user->uid);
    }
    $fids = $select->execute()->fetchAllKeyed(0, 0);
    
    return feeds_load($fids, $reload);
  }
  
}

/**
 * Save a feed.
 *
 * @param $feed
 *   The feed to save.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_save(&$feed, $type = NULL, DatabaseTransaction $transaction = NULL) {
  global $user;
  
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (!isset($feed->type)) {
      if (!$type) {
        $type = FEED_TYPE_CREATED;
      }
      if (!in_array($type, array(FEED_TYPE_CREATED, FEED_TYPE_IMPORTED, FEED_TYPE_SUBSCRIBED))) {
        throw new Exception('Feed type must be a valid type');
      }
      $feed->type = $type;
    }
    
    $feed->updated_at = REQUEST_TIME;
    if (!isset($feed->created_at)) {
      $feed->created_at = REQUEST_TIME;
    }
    
    if (!isset($feed->uid)) {
      $feed->uid = $user->uid;
    }
    drupal_alter('feed_save', $feed);
    
    if (!$status = drupal_write_record('feed', $feed, isset($feed->fid) ? 'fid' : array())) {
      throw new Exception('Could not save feed record');
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
 * Deletes a feed from the database.
 *
 * @param $feed
 *   The feed to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_delete($feed, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed->fid)) {
      $fiids = db_select('feed_item', 'fi')
        ->fields('fi', array('fiid'))
        ->condition('fid', $feed->fid)
        ->execute()->fetchAllKeyed(0,0);
        
      if (!empty($fiids) && !feed_items_delete($fiids, $transaction)) {
        throw new Exception('Unable to delete feed items');
      }
      
      $ffids = db_select('feed_field', 'ff')
        ->fields('ff', array('ffid'))
        ->condition('fid', $feed->fid)
        ->execute()->fetchAllKeyed(0,0);
        
      if (!empty($ffids) && !feed_fields_delete($ffids, $transaction)) {
        throw new Exception('Unable to delete a feed field');
      }
      
      db_delete('feed_item_subscription')
        ->condition('fid', $feed->fid)
        ->execute();
        
      db_delete('feed')
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
 * Configure feed object/ensure defaults
 * 
 * @param $feed
 * 	 A feed object.
 */
function feed_defaults($feed = NULL) {
  $feed = (object) $feed;

  if (!isset($feed->data)) {
    $feed->data = array();
  }
  return $feed;
}