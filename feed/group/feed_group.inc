<?php 

/**
 * Load a feed group.
 *
 * @param $fgid
 *   The feed group id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed group.
 */
function feed_group_load($fgid, $reload = FALSE) {
  $feed_groups = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_groups)) {
    $feed_groups = array();
  }
  if ($reload || !isset($feed_groups[$fgid])) {
    $feed_groups[$fgid] = db_query('SELECT * FROM {feed_group} WHERE fgid = :fgid', array(':fgid' => $fgid))->fetchObject();
  }
  return $feed_groups[$fgid];
}

/**
 * Return an array of all feed group objects.
 *
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed group objects, indexed by fgid.
 */
function feed_group_load_all($reload = FALSE) {
  $feed_groups = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_groups)) {
    $feed_groups = db_query('SELECT * FROM {feed_group} WHERE 1 = 1')->fetchAllAssoc('fgid');
  }
  return $feed_groups;
}

/**
 * Save a feed group.
 *
 * @param $feed_group
 *   The feed group to save.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_group_save(&$feed_group, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (!$status = drupal_write_record('feed_group', $feed_group, isset($feed_group->fgid) ? 'fgid' : array())) {
      throw new Exception('Could not save feed group record');
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
 * Deletes a feed group from the database.
 *
 * @param $feed_group
 *   The feed group to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_group_delete($feed_group, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_group->fgid)) {
      db_delete('feed_user_group')
        ->condition('fgid', $feed_group->fgid)
        ->execute();
      
      db_delete('feed_group')
        ->condition('fgid', $feed_group->fgid)
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
 * Configure feed group object/ensure defaults
 * 
 * @param $feed_group
 *   A feed group object.
 */
function feed_group_defaults($feed_group = NULL) {
  $feed_group = (object) $feed_group;
  
  if (!isset($feed_group->name)) $feed_group->name = '';
  if (!isset($feed_group->description)) $feed_group->description = '';
  
  return $feed_group;
}