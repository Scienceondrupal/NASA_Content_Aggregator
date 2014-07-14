<?php

/**
 * Load a feed widget.
 *
 * @param $fwid
 *   The feed widget id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An object describing the feed widget.
 */
function feed_widget_load($fwid, $reload = FALSE) {
  $feed_widgets = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_widgets)) {
    $feed_widgets = array();
  }
  if ($reload || !isset($feed_widgets[$fwid])) {
    $feed_widgets[$fwid] = db_query('SELECT * FROM {feed_widget} WHERE fwid = :fwid', array(':fwid' => $fwid))->fetchObject();

    if (is_string($feed_widgets[$fwid]->configuration) && $config = @unserialize($feed_widgets[$fwid]->configuration)) {
      $feed_widgets[$fwid]->configuration = $config;
    }
  }
  return $feed_widgets[$fwid];
}

/**
 * Return an array of all feed widget objects belonging to feed
 * 
 * @param $fid
 *   The feed id to load feed widget objects for.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed widget objects, indexed by fwid.
 */
function feed_widget_load_all_feed($fid, $reload = FALSE) {
  $feed_user_widgets = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_user_widgets[$fid])) {
    $feed_user_widgets[$fid] = db_select('feed_widget', 'fw')
      ->fields('fw')
      ->condition('fw.fid', $fid)
      ->execute()->fetchAllAssoc('fwid');
      
    foreach ($feed_user_widgets[$fid] as &$feed_widget) {
      if (is_string($feed_widget->configuration) && $config = @unserialize($feed_widget->configuration)) {
        $feed_widget->configuration = $config;
      }
    }
  }
  return $feed_user_widgets[$fid];
}

/**
 * Return an array of all feed widget objects belonging to user
 * 
 * @param $uid
 *   The User id to load feed widget objects for.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed widget objects, indexed by fwid.
 */
function feed_widget_load_all_user($uid = NULL, $reload = FALSE) {
  global $user;
  
  if (empty($uid)) {
    $uid = $user->uid;
  }
  $feed_user_widgets = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_user_widgets)) {
    $feed_user_widgets = db_select('feed_widget', 'fw')
      ->fields('fw')
      ->condition('fw.uid', $uid)
      ->execute()->fetchAllAssoc('fwid');
      
    foreach ($feed_user_widgets as &$feed_widget) {
      if (is_string($feed_widget->configuration) && $config = @unserialize($feed_widget->configuration)) {
        $feed_widget->configuration = $config;
      }
    }
  }
  return $feed_user_widgets;
}

/**
 * Return an array of all feed widget objects.
 *
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 *   
 * @return
 *   An array of all feed widget objects, indexed by fwid.
 */
function feed_widget_load_all($reload = FALSE) {
  $feed_user_widgets = drupal_static(__FUNCTION__);

  if ($reload || !isset($feed_user_widgets)) {
    $feed_user_widgets = db_query('SELECT * FROM {feed_widget} WHERE 1 = 1')->fetchAllAssoc('fwid');
    
    foreach ($feed_user_widgets as &$feed_widget) {
      if (is_string($feed_widget->configuration) && $config = @unserialize($feed_widget->configuration)) {
        $feed_widget->configuration = $config;
      }
    }
  }
  return $feed_user_widgets;
}

/**
 * Save a feed widget.
 *
 * @param $feed_widget
 *   The feed widget to save.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 *
 * @return
 *   Status flag indicating outcome of the operation.
 */
function feed_widget_save(&$feed_widget, DatabaseTransaction $transaction = NULL) {
  global $user;
  
  if (!$transaction) {
    $transaction = db_transaction();
  }
  
  try {
	  if (empty($feed_widget->uid)) {
	    $feed_widget->uid = $user->uid;
	  }
    if (!$status = drupal_write_record('feed_widget', $feed_widget, isset($feed_widget->fwid) ? 'fwid' : array())) {
      throw new Exception('Could not save feed widget record');
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
 * Deletes a feed widget from the database.
 *
 * @param $feed_widget
 *   The feed widget to delete, as an object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_widget_delete($feed_widget, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (isset($feed_widget->fwid)) {
      db_delete('feed_widget')
        ->condition('fwid', $feed_widget->fwid)
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
 * Configure feed widget object/ensure defaults
 * 
 * @param $feed_widget
 * 	 A feed widget object.
 */
function feed_widget_defaults($feed_widget = NULL) {
  $feed_widget = (object) $feed_widget;
  
  if (!isset($feed_widget->name)) $feed_widget->name = '';
  if (!isset($feed_widget->configuration)) $feed_widget->configuration = array();
  
  return $feed_widget;
}