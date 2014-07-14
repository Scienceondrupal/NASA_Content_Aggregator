<?php 

/**
 * Feed group overview page callback
 */
function feed_group_overview() {
  $build = array();
  $build['feed_links'] = array(
    '#theme' => 'links',
    '#attributes' => array('class' => array('feed-links')),
    '#links' => array(
      'add_group' => array(
        'title' => 'Add New Source',
        'href' => 'feed/source/add',
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
    ),
  );
  
  $build['feed_message'] = array(
    '#children' => '<div class="feed-message"></div>',
  );
  
  $build['feed_groups'] = drupal_get_form('feed_group_overview_form');
  
  return $build;
}

/**
 * Feed group overview form
 */
function feed_group_overview_form($form, &$form_state) {
  $feed_groups = feed_group_load_all();
  
  $form['feed_group'] = array();
  $form['feed_group']['#type'] = 'feed_user_group';
  $form['feed_group']['#tree'] = TRUE;
  
  $users = array();
  
  $feed_group_options = array('none' => t('None'));
  foreach ($feed_groups as $feed_group) {
    $feed_group_options[$feed_group->fgid] = $feed_group->name;
  }
  
  foreach ($feed_groups as $feed_group) {
    $form['feed_group'][$feed_group->fgid] = array('#tree' => TRUE);
    
    $form['feed_group'][$feed_group->fgid]['name'] = array('#markup' => $feed_group->name);
    $form['feed_group'][$feed_group->fgid]['description'] = array('#markup' => $feed_group->description);
    $form['feed_group'][$feed_group->fgid]['weight'] = array('#type' => 'hidden', '#value' => $feed_group->weight);
    $form['feed_group'][$feed_group->fgid]['pfgid'] = array('#type' => 'hidden', '#value' => $feed_group->pfgid);
    $form['feed_group'][$feed_group->fgid]['fgid'] = array('#type' => 'hidden', '#value' => $feed_group->fgid);
    $form['feed_group'][$feed_group->fgid]['edit'] = array(
      '#type' => 'link',
      '#title' => t('Edit'),
      '#href' => 'feed/source/' . $feed_group->fgid,
    );
    $form['feed_group'][$feed_group->fgid]['delete'] = array(
      '#type' => 'link',
      '#title' => t('Delete'),
      '#href' => 'feed/source/' . $feed_group->fgid . '/delete',
    );
    
    $group_users = feed_group_users_load($feed_group);
    
    $users = array_merge($users, array_keys($group_users));
    
    $form['feed_group'][$feed_group->fgid]['users'] = array('#tree' => TRUE);
    foreach ($group_users as $user) {
      $form['feed_group'][$feed_group->fgid]['users'][$user->uid] = array('#tree' => TRUE);
      $form['feed_group'][$feed_group->fgid]['users'][$user->uid]['username'] = array('#markup' => $user->name);
      $form['feed_group'][$feed_group->fgid]['users'][$user->uid]['fgid'] = array(
        '#type' => 'select',
        '#default_value' => $feed_group->fgid,
        '#options' => $feed_group_options,
        '#attributes' => array('class' => array('feed-group-select', 'feed-group-' . $feed_group->fgid)),
      );
    }
  }
  
  $userpool = feed_users('eosdis admin feed');
  
  $form['feed_group']['none'] = array('#tree' => TRUE);
  $form['feed_group']['none']['name'] = array('#markup' => t('None'));
  $form['feed_group']['none']['description'] = array('#markup' => t('These users are not assigned to a source.'));
  $form['feed_group']['none']['weight'] = array('#type' => 'hidden', '#value' => 50);
  $form['feed_group']['none']['pfgid'] = array('#type' => 'hidden', '#value' => '');
  $form['feed_group']['none']['fgid'] = array('#type' => 'hidden', '#value' => '');
  $form['feed_group']['none']['users'] = array('#tree' => TRUE);
  
  foreach ($userpool as $user) {
    if (!in_array($user->uid, $users)) {
      $form['feed_group']['none']['users'][$user->uid]['username'] = array('#markup' => $user->name);
      $form['feed_group']['none']['users'][$user->uid]['fgid'] = array(
        '#type' => 'select',
        '#default_value' => 'none',
        '#options' => $feed_group_options,
        '#attributes' => array('class' => array('feed-group-select', 'feed-group-none')),
      );
    }
  }
  
  $form['#attributes'] = array('class' => array('feed-group-users-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save User Assignments'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Feed group overview submit handler
 */
function feed_group_overview_form_submit($form, &$form_state) {
  if (!empty($form_state['values']['feed_group'])) {
    $modified_users = array();
    
    foreach ($form_state['values']['feed_group'] as $old_fgid => $values) {
      if (!empty($values['users']) && $users = $values['users']) {
        foreach ($users as $uid => $user) {
          $fgid = $user['fgid'];
          
          if ($old_fgid != $fgid) {
            if (!isset($modified_users[$fgid])) {
              $modified_users[$fgid] = array();
            }
            $modified_users[$fgid][] = $uid;
          }
        }
      }
    }
    
    $success = TRUE;
    foreach ($modified_users as $fgid => $uids) {
      if ($fgid == 'none') {
        if (!feed_group_users_remove_primary($uids) && $success) {
          $success = FALSE;
        }
      }
      elseif (is_numeric($fgid) && $feed_group = feed_group_load($fgid)) {
        if (!feed_group_users_set_primary($uids, $feed_group) && $success) {
          $success = FALSE;
        }
      }
    }
      
    if ($success) {
      drupal_set_message(t('Successfully saved user assignments'), 'status');
    }
    else {
      drupal_set_message(t('Error saving user assignments'), 'error');
    }
  }
}


/**
 * Feed group form; 
 */
function feed_group_form($form, &$form_state, $feed_group = NULL) {
  $form_state['#feed_group'] = feed_group_defaults($feed_group);
  
  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['#feed_group']->name) ? $form_state['#feed_group']->name : '',
    '#description' => t('The human-readable name of this feed source. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 60,
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['#feed_group']->description) ? $form_state['#feed_group']->description : '',
    '#description' => t('A short description of this source.'),
  );
  
  $form['#attributes'] = array('class' => array('feed-group-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed source'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Feed group form submit handler; 
 */
function feed_group_form_submit(&$form, &$form_state) {
  $feed_group =& $form_state['#feed_group'];
  $feed_group->name = trim($form_state['values']['name']);
  $feed_group->description = trim($form_state['values']['description']);

  $status = feed_group_save($feed_group);
  
  if ($status == SAVED_UPDATED) {
    drupal_set_message(t('The feed source %name has been updated.', array('%name' => $feed_group->name)));
  }
  elseif ($status == SAVED_NEW) {
    drupal_set_message(t('The feed source %name has been added.', array('%name' => $feed_group->name)));
  }
  $form_state['redirect'] = 'feed/source';
}

/**
 * Delete feed group form;
 */
function feed_group_delete_form($form, &$form_state, $feed_group) {
  $form_state['#feed_group'] = feed_group_defaults($feed_group);

  $form['#attributes'] = array('class' => array('feed-form'));
  
  return confirm_form(
    $form,
    t('Delete feed source?'),
    'feed/source',
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete feed source ":name". This action cannot be undone and user assignments for this source will be removed', array(':name' => $feed_group->name)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed group form submit handler;
 */
function feed_group_delete_form_submit(&$form, &$form_state) {
  $feed_group = $form_state['#feed_group'];
  
  if (feed_group_delete($feed_group)) {
    drupal_set_message(t('Feed source :name was deleted.', array(':name' => $feed_group->name)));
  } 
  else {
    drupal_set_message(t('Could not delete feed source :name.', array(':name' => $feed_group->name)), 'error');
  }
  $form_state['redirect'] = 'feed/source';
}