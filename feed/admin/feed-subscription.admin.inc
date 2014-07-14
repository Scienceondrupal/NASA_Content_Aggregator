<?php 

/**
 * Menu callback; Shows the feed list.
 */
function feed_subscription_overview() {
  drupal_set_title('My Subscriptions');
  
  $feeds = feed_load_all(FEED_TYPE_SUBSCRIBED);
  
  $header = array(
    array('data' => t('Title'), 'class' => array('feed-col-1')), 
    array('data' => t('Manage Subscription'), 'colspan' => '4', 'class' => array('feed-col-2')), 
    array('data' => t('Manage Subscribed Items'), 'colspan' => '3', 'class' => array('feed-col-3'))
  );
  $rows = array();

  foreach ($feeds as $feed) {
    if (feed_access($feed)) {
      $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
    
      $row = array();
      $row[] = array('data' => l($title, "feed/{$feed->fid}/format/web"), 'class' => array('feed-col-1'));
      $row[] = array('data' => l(t('RSS'), "feed/{$feed->fid}/format/rss"), 'class' => array('feed-col-2'));
      $row[] = array('data' => l(t('widget'), "feed/{$feed->fid}/widget"), 'class' => array('feed-col-3'));
      $row[] = array('data' => l(t('edit'), "feed/subscriptions/{$feed->fid}/edit"), 'class' => array('feed-col-4'));
      $row[] = array('data' => l(t('delete'), "feed/subscriptions/{$feed->fid}/delete"), 'class' => array('feed-col-5'));
      $row[] = array('data' => l(t('view subscription'), "feed/subscriptions/{$feed->fid}/feed-items"), 'class' => array('feed-col-6'));
      $row[] = array('data' => l(t('add subscription item'), "feed/subscriptions/{$feed->fid}/feed-items/add"), 'class' => array('feed-col-7'));
      $row[] = array('data' => l(t('auto subscribe'), "feed/subscriptions/{$feed->fid}/filter"), 'class' => array('feed-col-8'));
  
      $rows[] = $row;
    }
  }
  
  $build = array();
  
  $build['feed_links'] = array(
    '#theme' => 'links',
    '#attributes' => array('class' => array('feed-links')),
    '#links' => array(
      'add_subscription' => array(
        'title' => 'Add New Subscription',
        'href' => 'feed/subscriptions/add',
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
    ),
  );
  $build['feed_subscription_overview'] = array(
    '#theme' => 'feed_block',
    '#attributes' => array('id' => 'feed-subscription-overview'),
    '#title' => 'Subscription Feed',
    '#content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#empty' => t('No feed subscriptions. <a href="@link">Add feed subscription</a>.', array(
      	'@link' => url("feed/subscriptions/add"),
      )),
    ),
  );
  return $build;
}

/**
 * Manage feed form;
 */
function feed_subscription_form($form, &$form_state, $feed = NULL) {
  $form_state['#feed'] = feed_defaults($feed);
  $form_state['#feed_fields'] = !empty($feed->fid) ? feed_load_fields($feed, FALSE) : default_feed_fields();
  
  $form['feed'] = array(
    '#type' => 'feed_fieldset',
    '#title' => 'Create Subscription Feed',
    '#tree' => TRUE,
    '#attributes' => array(
      'class' => array('feed-create-container'),
    )
  );
  
  // Run handlers to attach feed widgets
  $form['feed']['feed_fields_required'] = array('#type' => 'container');
  $form['feed']['feed_fields'] = array('#type' => 'container');
  
  foreach ($form_state['#feed_fields'] as $ffid => &$feed_field) {
    $key = !empty($feed_field->is_required) ? 'feed_fields_required' : 'feed_fields';
    
    $feed_field_values = !empty($form_state['#feed']->fid) ? feed_field_values_load($form_state['#feed'], $feed_field) : array();
    $form['feed'][$key][$ffid] = array(
      '#type' => 'feed_field_widget',
      '#feed_field' => &$feed_field,
      '#feed_field_values' => $feed_field_values,
    );
  }
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save subscription'),
    '#weight' => 40,
    '#attributes' => array('class' => array('feed-submit')),
  );
  return $form;
}

/**
 * Manage feed form submit handler;
 */
function feed_subscription_form_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  $is_new = !empty($feed->fid);
  
  $feed->type = FEED_TYPE_SUBSCRIBED;
  if ($status = feed_save($feed)) {
    foreach (array('feed_fields_required', 'feed_fields') as $widget_key) {
      foreach (element_children($form['feed'][$widget_key]) as $key) {
        $feed_field_widget  = $form['feed'][$widget_key][$key];
      
        if (isset($feed_field_widget['#feed_field_values'], $feed_field_widget['#feed_field'])) {
          if (!isset($feed_field_widget['#feed_field']->ffid) && !feed_field_save($feed_field_widget['#feed_field'], $feed, FALSE)) {
            drupal_set_message(t('Error saving value for field %field.', array('%field' => $feed_field_widget['#feed_field']->name)), 'error');
          }
          
          if (isset($feed_field_widget['#feed_field']->ffid) && !feed_field_values_save($feed_field_widget['#feed_field_values'], $feed, $feed_field_widget['#feed_field'])) {
            drupal_set_message(t('Error saving value for field %field.', array('%field' => $feed_field_widget['#feed_field']->name)), 'error');
          }
        }
      }
    }
    $title = !empty($feed->fid) ? feed_field_values_tagname_format($feed->fid, 'title', 'text') : '';
    
    if ($status == SAVED_UPDATED) {
      drupal_set_message(t('The feed subscription "%title" has been updated.', array('%title' => $title)));
    }
    elseif ($status == SAVED_NEW) {
      drupal_set_message(t('The feed subscription "%title" has been added.', array('%title' => $title)));
    }
    $form_state['redirect'] = 'feed/subscriptions';
  }
}

/**
 * Delete subscription form;
 */
function feed_subscription_delete_form($form, &$form_state, $feed) {
  if ($feed->type != FEED_TYPE_SUBSCRIBED) {
    drupal_goto("feed/subscriptions");
  }
  $form_state['#feed'] = feed_defaults($feed);
  $form['#attributes'] = array('class' => array('feed-form'));
  
  $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
  return confirm_form(
    $form,
    t('Delete subscription?'),
    'feed/subscriptions',
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete subscription ":title". This action cannot be undone and all items will be unsubscribed', array(':title' => $title)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete subscription form submit handler;
 */
function feed_subscription_delete_form_submit(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  
  $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
  if (feed_delete($feed)) {
    drupal_set_message(t('Subscription ":title" was deleted.', array(':title' => $title)));
  } 
  else {
    drupal_set_message(t('Could not delete subscription ":title".', array(':title' => $title)), 'error');
  }
  $form_state['redirect'] = 'feed/subscriptions';
}

/**
 * Feed item subscription overview form;
 */
function feed_item_subscription_overview_form($form, &$form_state, $feed) {
  if (!isset($feed->fid)) {
    drupal_goto('feed/subscriptions');
  }
  $form_state['#feed'] = feed_defaults($feed);
  
  $pager = new FeedItemPager();
  $pager->hasFeedSubscribed($feed->fid);
  
  $form['select'] = array(
    '#type' => 'feed_item_select',
    '#id' => 'feed-item-select-filtered',
    '#empty' => t('No feed items subscribed. <a href="' . url('feed/subscriptions/' . $feed->fid . '/feed-items/add') . '">Subscribe to feed items</a>.'),
    '#title' => 'Subscribed Items',
    '#pager' => $pager,
  );
  
  $form['action'] = array('#type' => 'actions');
  $form['action']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Unsubscribe'),
    '#attributes' => array('class' => array('feed-item-submit')),
  );
  
  $form['#attributes'] = array('class' => array('feed-form'));
  return $form;
}

/**
 * Feed item subscription overview form submit handler;
 */
function feed_item_subscription_overview_form_submit($form, &$form_state) {
  $feed_items = $form['select']['#feed_item_selected'];
  
  if (feed_unsubscribe_feed_items($form_state['#feed'], $feed_items)) {
    drupal_set_message('Feed items successfully unsubscribed.');
  }
  else {
    drupal_set_message('Error unsubscribing feed items.', 'error');
  }
}

/**
 * Feed item subscription form;
 */
function feed_item_subscription_form($form, &$form_state, $feed) {
  if (!isset($feed->fid)) {
    drupal_goto('feed/subscription');
  }
  $form_state['#feed'] = feed_defaults($feed);
  
  if (empty($form_state['#pager'])) {
    $form_state['#pager'] = new FeedItemPager(20);
  }
  
  $form['filter'] = array(
    '#type' => 'feed_item_filter',
    '#pager' => &$form_state['#pager'],
    '#ajax' => array(
      'callback' => 'feed_item_subscription_filter_ajax',
      'wrapper' => 'feed-item-select-filtered',
      'method' => 'replace',
    ),
  );
  
  $form['select'] = array('#tree' => TRUE);
  $form['select']['result'] = array(
    '#type' => 'feed_item_select',
    '#id' => 'feed-item-select-filtered',
    '#pager' => &$form_state['#pager'],
    '#title' => 'Subscription Items',
    '#empty' => 'No results match your criteria.',
    '#after_build' => array('form_build_feed_item_subscription_select'),
    '#links' => FALSE,
  );
  
  $form['select']['actions'] = array(
  	'#type' => 'actions', 
  	'#attributes' => array(
  		'class' => array('feed-subscription-submit')
    ),
  );
  $form['select']['actions']['subscribe'] = array(
    '#type' => 'submit',
    '#value' => t('Subscribe'),
  );
  $form['#attributes'] = array('class' => array('feed-form'));
  
  return $form;
}

/**
 * Feed item subscription form after build handler.
 */
function form_build_feed_item_subscription_select(&$element, &$form_state) {
  foreach (feed_load_subscribed_feed_items($form_state['#feed']) as $feed_item) {
    if (isset($element['feed_items'][$feed_item->fiid])) {
      $element['feed_items'][$feed_item->fiid]['checkbox']['#attributes']['disabled'] = TRUE;
      $element['feed_items'][$feed_item->fiid]['checkbox']['#checked'] = TRUE;
    }
  }
  return $element;
}

/**
 * Ajax callback for feed_item_filter element.
 */
function feed_item_subscription_filter_ajax($form, &$form_state) {
  return $form['select']['result'];
}

/**
 * Feed item subscription form submit handler;
 */
function feed_item_subscription_form_submit(&$form, &$form_state) {
  $feed =& $form_state['#feed'];
  
  switch ($form_state['values']['op']) {
    case $form['filter']['filter']['submit']['#value']:
      $form_state['rebuild'] = TRUE;
    break;
    
    case $form['select']['actions']['subscribe']['#value']:
      $feed_items = $form['select']['result']['#feed_item_selected'];

      if (feed_subscribe_feed_items($feed, $feed_items)) {
        drupal_set_message('Successfully subscribed to feed items');
      }
      else {
        drupal_set_message('Error subscribing to feed items', 'error');
      }
      $form_state['rebuild'] = TRUE;
    break;
  }
}

/**
 * Feed subscribe form; Subscribes to a single feed
 */
function feed_subscribe_form($form, &$form_state, $feed = NULL) {
  global $user;
  
  $pager = new FeedItemPager;
  $pager->hasFeedCreated($feed->fid);
  
  $title = feed_field_values_tagname_format($feed->fid, 'title', 'text');
  
  if ($pager->count() == 0) {
    drupal_set_message(t('The feed ":feed" does not have any created feed items.', array(':feed' => $title)), 'status');
    drupal_goto('feed/all');
  }
  if (!isset($form_state['#feed_subscribe'])) {
    $form_state['#feed_subscribe'] = feed_defaults($feed);
  }
  
  if (isset($form_state['#feed_subscribe_method']) && $form_state['#feed_subscribe_method'] == 'create') {
    $form = feed_subscription_form($form, $form_state);
  }
  else {
    $feed_options = array();
    foreach (feed_load_all(FEED_TYPE_SUBSCRIBED, $user) as $feed_option) {
      $option_title = feed_field_values_tagname_format($feed_option->fid, 'title', 'text');
      $feed_options[$feed_option->fid] = strlen($option_title) > 50 ? substr($option_title, 0, 47) . '...' : $option_title;
    }
    $subscribe_options = array();
    if (!empty($feed_options)) {
      $subscribe_options['add'] = 'Add selected feed item(s) to an existing subscription.';
    }
    $subscribe_options['create'] = 'Create a new subscription with the selected feed item(s).';
    
    $form['subscribe'] = array('#tree' => TRUE);
    $form['subscribe']['method'] = array(
      '#type' => 'radios',
      '#required' => TRUE,
      '#title' => t('Subscription method'),
      '#options' => $subscribe_options,
      '#attributes' => array('class' => array('feed-subscribe-method')),
    );
    
    if (!empty($feed_options)) {
      $form['subscribe']['feed'] = array(
        '#type' => 'select',
        '#title' => t('Subscribe selected items to this feed subscription'),
        '#options' => $feed_options,
        '#empty_option' => '- Select Feed -',
        '#states' => array(
          'visible' => array(
            'input.feed-subscribe-method' => array('value' => 'add'),
          ),
        ),
      );
    }
    
    $form['select']['feed_items'] = array(
    	'#type' => 'feed_item_select',
      '#pager' => $pager,
      '#empty' => 'No feeds items to list',
      '#title' => 'Subscribe to Feed Items',
      '#links' => array(),
    );
    $form['subscribe']['actions'] = array('#type' => 'actions');
    $form['subscribe']['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Subscribe'),
    );
  }
  return $form;
}

/**
 * Feed subscription form submit handler
 */
function feed_subscribe_form_submit(&$form, &$form_state) {
  if (isset($form['subscribe']) && $form_state['values']['op'] == $form['subscribe']['actions']['submit']['#value']) {
    $form_state['#feed_item_subscribe'] = $form['select']['feed_items']['#feed_item_selected'];
    $form_state['#feed_subscribe_method'] = $form_state['values']['subscribe']['method'];

    if ($form_state['values']['subscribe']['method'] == 'add') {
      $feed = feed_load($form_state['values']['subscribe']['feed']);
      
      if (!$feed || $feed->type != FEED_TYPE_SUBSCRIBED) {
        form_error($form['subscribe']['feed'], 'Invalid feed selected, feed does not exists.');
      }
      elseif (!feed_access($feed)) {
        form_error($form['subscribe']['feed'], 'Invalid feed selected, you do not have access to this feed.');
      }
      elseif (feed_subscribe_feed_items($feed, $form_state['#feed_item_subscribe'])) {
        drupal_set_message('Successfully subscribed to feed.');
      }
      else {
        drupal_set_message('Error subscribing to feed.', 'error');
      }
      $form_state['redirect'] = 'feed/subscriptions';
    }
    else {
      $form_state['rebuild'] = TRUE;
    }
  }
  elseif (isset($form['feed'], $form['actions']['submit']['#value']) && $form_state['values']['op'] == $form['actions']['submit']['#value']) {
    if (isset($form_state['#feed_item_subscribe'])) {
      feed_subscription_form_submit($form, $form_state);
      $errors = form_get_errors();
      
      if (empty($errors) && isset($form_state['#feed']->fid)) {
        feed_subscribe_feed_items($form_state['#feed'], $form_state['#feed_item_subscribe']);
      }
    }
    else {
      unset($form_state['#feed_subscribe_method']);
      $form_state['rebuild'] = TRUE;
    }
  }
}