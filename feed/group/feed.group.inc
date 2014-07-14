<?php 

/**
 * Theming function for feed user group element
 * @param $variables
 */
function theme_feed_user_group($variables) {
  $elements = $variables['elements'];
  
  // Headers.
  $header = array(
    array('data' => t('Username'), 'class' => array('feed-group-header-name')),
    array('data' => t('Source'), 'class' => array('feed-group-header-description')),
    array('data' => t('Manage'), 'class' => array('feed-group-header-links')),
  );
  
  $rows = array();
  
  drupal_add_js(drupal_get_path('module', 'feed') . '/js/feed.group.js');
  
  // Field Rows. 
  foreach (element_children($elements, TRUE) as $fgid) {
    drupal_add_tabledrag('feedGroupUserDrag', 'match', 'sibling', 'feed-group-select', 'feed-group-' . $fgid, NULL, FALSE);
    
    $group = $elements[$fgid];
    
    $links = array();
    if (!empty($group['edit'])) {
      $links[] = $group['edit'];
    }
    if (!empty($group['delete'])) {
      $links[] = $group['delete'];
    }
    
    $rows[] = array(
      'class' => array('group-title', 'group-title-' . $fgid),
      'data' => array(
        array('data' => array($group['name']), 'class' => array('feed-group-name')),
        array('data' => array($group['description']), 'class' => array('feed-group-description')),
        array('data' => $links, 'class' => array('feed-group-links')),
      ),
    );
    
    $users = element_children($group['users'], TRUE);
    
    $rows[] = array(
      'class' => array(
        'group-message', 
        'group-' . $fgid . '-message',
        'group-title-' . $fgid,
        (empty($users) ? 'group-empty' : 'group-populated'),
      ),
      'data' => array(
        array('data' => array('#markup' => t('No users in this group')), 'colspan' => '3'),
      ),
    );
    
    foreach ($users as $name) {
      $user = $group['users'][$name];
      
      $rows[] = array(
        'class' => array('draggable', 'feed-group-user'),
        'data' => array(
          array('data' => array($user['username'])),
          array('data' => array($user['fgid']), 'colspan' => '2'),
        ),
      );
    }
  }
  
  return theme('feed_block', array(
    'attributes' => array('id' => 'feed-groups-block'),
    'title' => 'Feed Groups',
    'content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No groups created.'),
      '#attributes' => array(
        'id' => 'feedGroupUserDrag',
      ),
    ),
  ));
}

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
      throw new Exception('Could not save feed source record');
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

/**
 * Load users in a feed group
 * 
 * @param $feed_group
 *   A feed group object.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 */
function feed_group_users_load($feed_group, $primary = TRUE, $reload = FALSE) {
  $feed_group_users = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_group->fgid)) {
    return array();
  }
  if (!isset($feed_group_users)) {
    $feed_group_users = array();
  }
  $index = $feed_group->fgid . ($primary ? '_primary' : '_default');
  
  if ($reload || !isset($feed_group_users[$index])) {
    $query = db_select('users', 'u');
    $query->join('feed_user_group', 'fug', 'fug.uid = u.uid');
    $query->fields('u');
    $query->condition('u.status', '1');
    $query->condition('fug.fgid', $feed_group->fgid);
    
    if ($primary) {
      $query->condition('fug.primary_group', '1');
    }
    $feed_group_users[$index] = $query->execute()->fetchAllAssoc('uid');
  }
  return $feed_group_users[$index];
}

/**
 * Remove primary groups for a set of users
 * 
 * @param $uids
 *   A user id or an array of user id.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_group_users_remove_primary($uids, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (!is_array($uids)) {
      $uids = array($uids);
    }
    
    if (!empty($uids)) {
      db_delete('feed_user_group')
        ->condition('uid', $uids)
        ->condition('primary_group', 1)
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
 * Gets primary group for feed users.
 * 
 * @param $uid
 *   A user id.
 * @param $reload (optional)
 *   A boolean to indicate whether to reload data from database.
 */
function feed_group_user_get_primary($uid, $reload = FALSE) {
  $feed_user_primaries = &drupal_static(__FUNCTION__);
  
  if (!isset($feed_user_primaries)) {
    $feed_user_primaries = array();
  }
  
  if ($reload || !isset($feed_user_primaries[$uid])) {
    $query = db_select('feed_user_group', 'fug');
    $query->join('feed_group', 'fg', 'fg.fgid = fug.fgid');
    $query->fields('fg');
    $query->condition('fug.primary_group', '1');
    $query->condition('fug.uid', $uid);
    $query->range(0, 1);
    
    $results = $query->execute()->fetchAllAssoc('fgid');
    
    if ($result = array_pop($results)) {
      $feed_user_primaries[$uid] = $result;
    }
  }
  return !empty($feed_user_primaries[$uid]) ? $feed_user_primaries[$uid] : NULL;
}

/**
 * Sets primary group for feed users.
 * 
 * @param $uids
 *   A user id or an array of user id.
 * @param $feed_group
 *   A feed group object.
 * @param $transaction (optional)
 *   A DatabaseTransaction object.
 */
function feed_group_users_set_primary($uids, $feed_group, DatabaseTransaction $transaction = NULL) {
  if (!$transaction) {
    $transaction = db_transaction();
  }

  try {
    if (!is_array($uids)) {
      $uids = array($uids);
    }
    if (isset($feed_group->fgid) && feed_group_users_remove_primary($uids, $transaction)) {
      $insert = db_insert('feed_user_group')->fields(array('fgid', 'uid', 'primary_group'));
      
      foreach ($uids as $uid) {
        $insert->values(array('fgid' => $feed_group->fgid, 'uid' => $uid, 'primary_group' => '1'));
      }
      $insert->execute();
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
 * Load user with a feed permission
 * @param $permission
 *   The permission string to filter users by.
 */
function feed_users($permission) {
  $rids_query = db_select('role_permission', 'rp');
  $rids_query->join('role', 'r', 'r.rid = rp.rid');
  $rids_query->fields('r', array('rid'));
  $rids_query->condition('rp.permission', $permission);
  $rids_query->condition('rp.module', 'feed');
  
  $rids = $rids_query->execute()->fetchAllKeyed(0,0);
  
  if (!empty($rids)) {
    $user_query = db_select('users', 'u');
    $user_query->join('users_roles', 'ur', 'ur.uid = u.uid');
    $user_query->fields('u');
    $user_query->condition('u.status', '1');
    $user_query->condition('ur.rid', $rids, 'IN');
    
    return $user_query->execute()->fetchAllAssoc('uid');
  }
  return array();
}