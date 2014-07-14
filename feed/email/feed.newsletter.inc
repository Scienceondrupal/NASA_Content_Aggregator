<?php

/**
 * Load all feed newsletter.
 *
 * @return
 *   An array of objects describing the feed newsletter indexed by fnid.
 */
function feed_newsletter_load_all() {
  $feed_newsletters = db_query('SELECT * FROM {feed_newsletter} WHERE 1 = 1')->fetchAllAssoc('fnid');
    
  foreach ($feed_newsletters as &$feed_newsletter) {
    if (is_string($feed_newsletter->configuration) && $config = @unserialize($feed_newsletter->configuration)) {
      $feed_newsletter->configuration = $config;
    }
  }
}

/**
 * Load feed newsletter for a user.
 *
 * @param $user
 *   A user object to load newsletter, or current user if none provided
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of objects describing the feed newsletter indexed by fnid.
 */
function feed_newsletter_load_all_user($user = NULL, $reload = FALSE) {
  if (!$user) {
    global $user;
  }
  $feed_newsletters = &drupal_static(__FUNCTION__);
  
  if ($reload || !isset($feed_newsletters[$user->uid])) {
    $feed_newsletters[$user->uid] = db_select('feed_newsletter', 'fn')
      ->fields('fn')
      ->condition('fn.uid', $user->uid)
      ->execute()->fetchAllAssoc('fnid');
      
    foreach ($feed_newsletters[$user->uid] as &$feed_newsletter) {
      if (is_string($feed_newsletter->configuration) && $config = @unserialize($feed_newsletter->configuration)) {
        $feed_newsletter->configuration = $config;
      }
    }
  }
  return $feed_newsletters[$user->uid];
}

/**
 * Load a feed newsletter.
 *
 * @param $fnid
 *   The feed newsletter id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed newsletter.
 */
function feed_newsletter_load($fnid, $reload = FALSE) {
  $feed_newsletters = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_newsletters)) {
    $feed_newsletters = array();
  }
  if ($reload || !isset($feed_newsletters[$fnid])) {
    $feed_newsletters[$fnid] = db_select('feed_newsletter', 'fn')
      ->fields('fn')
      ->condition('fn.fnid', $fnid)
      ->execute()->fetchObject();

    if (is_string($feed_newsletters[$fnid]->configuration) && $config = @unserialize($feed_newsletters[$fnid]->configuration)) {
      $feed_newsletters[$fnid]->configuration = $config;
    }
  }
  return $feed_newsletters[$fnid];
}

/**
 * Save a feed newsletter.
 *
 * @param $feed_newsletter
 *   The feed newsletter object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_newsletter_save($feed_newsletter, DatabaseTransaction $transaction = NULL) {
  global $user;
  
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  try {
    $feed_newsletter->uid = $user->uid;

    if (!$status = drupal_write_record('feed_newsletter', $feed_newsletter, isset($feed_newsletter->fnid) ? 'fnid' : array())) {
      throw new Exception('Could not save a feed newsletter record');
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
 * Deletes a feed newsletter from the database.
 *
 * @param $feed_newsletter
 *   The feed newsletter to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_newsletter_delete($feed_newsletter, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_newsletter->fnid)) {
      db_delete('feed_newsletter')
        ->condition('fnid', $feed_newsletter->fnid)
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
 * Configure feed newsletter object/ensure defaults
 * 
 * @param $feed_newsletter
 *   A feed newsletter object.
 */
function feed_newsletter_defaults($feed_newsletter = NULL) {
  $feed_newsletter = (object) $feed_newsletter;
  
  if (!isset($feed_newsletter->name)) $feed_newsletter->name = '';
  if (!isset($feed_newsletter->configuration)) $feed_newsletter->configuration = array();
  
  return $feed_newsletter;
}