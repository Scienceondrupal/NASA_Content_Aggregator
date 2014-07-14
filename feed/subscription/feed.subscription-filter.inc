<?php

/**
 * Load a feed subscription filter.
 *
 * @param $fsfid
 *   The feed subscription filter id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 * 
 * @return
 *   An object describing the feed subscription filter.
 */
function feed_subscription_filter_load($fsfid, $reload = FALSE) {
  return end(feed_subscription_filters_load(array($fsfid), $reload));
}


/**
 * Load feed subscription filters.
 *
 * @param $fsfids
 *   An array of feed subscription filter id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of object describing the feed subscription filter.
 */
function feed_subscription_filters_load(array $fsfids, $reload = FALSE) {
  $feed_subscription_filters = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_subscription_filters)) {
    $feed_subscription_filters = array();
  }
  $diff_fsfids = array_diff($fsfids, array_keys($feed_subscription_filters));
  if ($reload || !empty($diff_fsfids)) {
    $diff_feed_subscription_filters = db_select('feed_subscription_filter', 'fsf')
      ->fields('fsf')
      ->condition('fsf.fsfid', $diff_fsfids)
      ->execute()->fetchAllAssoc('fsfid');
      
    foreach ($diff_feed_subscription_filters as $diff_feed_subscription_filter) {
      if (is_string($diff_feed_subscription_filter->configuration) && $config = @unserialize($diff_feed_subscription_filter->configuration)) {
        $diff_feed_subscription_filter->configuration = $config;
      }
      $feed_subscription_filters[$diff_feed_subscription_filter->fsfid] = $diff_feed_subscription_filter;
    }
  }
  
  $results = array();
  foreach ($fsfids as $fsfid) {
    if (isset($feed_subscription_filters[$fsfid])) {
      $results[$feed_subscription_filters[$fsfid]->fid] = $feed_subscription_filters[$fsfid];
    }
  }
  return $results;
}

/**
 * Return an array of all feed subscription filter objects belonging to feed
 * 
 * @param $fid
 *   The feed id to load feed subscription filter objects for.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed subscription filter objects belonging to feed, indexed by fsfid.
 */
function feed_subscription_filter_load_all_feed($fid = NULL, $reload = FALSE) {
  $feed_subscription_filters = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_subscription_filters)) {
    $feed_subscription_filters = db_select('feed_subscription_filter', 'fsf')
      ->fields('fsf')
      ->condition('fsf.fid', $fid)
      ->execute()->fetchAllAssoc('fsfid');
      
    foreach ($feed_subscription_filters as $feed_subscription_filter) {
      if (is_string($feed_subscription_filter->configuration) && $config = @unserialize($feed_subscription_filter->configuration)) {
        $feed_subscription_filter->configuration = $config;
      }
    }
  }
  return $feed_subscription_filters;
}

/**
 * Save a feed subscription filter.
 *
 * @param $feed_subscription_filter
 *   The feed subscription filter to save.
 * @param $feed
 *   The feed object to assign the filter to.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_subscription_filter_save(&$feed_subscription_filter, $feed, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  try {
	  $feed_subscription_filter->fid = $feed->fid;
	  
    if (!$status = drupal_write_record('feed_subscription_filter', $feed_subscription_filter, isset($feed_subscription_filter->fsfid) ? 'fsfid' : array())) {
      throw new Exception('Could not save feed subscription filter record');
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
 * Deletes a feed subscription filter from the database.
 *
 * @param $feed_subscription_filter
 *   The feed subscription filter to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_subscription_filter_delete($feed_subscription_filter, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_subscription_filter->fsfid)) {
      db_delete('feed_subscription_filter')
        ->condition('fsfid', $feed_subscription_filter->fsfid)
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
 * Configure feed subscription filter object/ensure defaults
 * 
 * @param $feed_subscription_filter
 * 	 A feed subscription filter object.
 */
function feed_subscription_filter_defaults($feed_subscription_filter = NULL) {
  $feed_subscription_filter = (object) $feed_subscription_filter;
  
  if (!isset($feed_subscription_filter->name)) $feed_subscription_filter->name = '';
  if (!isset($feed_subscription_filter->configuration)) $feed_subscription_filter->configuration = array();
  
  return $feed_subscription_filter;
}