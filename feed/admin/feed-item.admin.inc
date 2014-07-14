<?php

/**
 * Feed item overview form;
 */
function feed_item_overview_form($form, &$form_state, $feed) {
  if (!isset($feed->fid)) {
    drupal_goto('feed');
  }
  if (!empty($form_state['#delete'])) {
    drupal_set_title('Delete feed items?');
    
    $form['delete_items'] = array('#tree' => TRUE);
    $delete_titles = array();
    foreach ($form_state['#delete'] as $feed_item) {
      $title = feed_item_field_values_tagname_format($feed_item->fiid, 'title', 'text');
      
      $form['delete_items'][$feed_item->fiid] = array('#type' => 'hidden', '#value' => $feed_item->fiid);
      $delete_titles[] = $title;
    }
    $question = '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . t('You are about to delete feed items @title', array('@title' => implode(', ', $delete_titles))) . '</div>';

    $form['#attributes']['class'][] = 'confirmation';
    $form['description'] = array('#markup' => $question);
  
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['delete_confirm'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
    );
    $form['actions']['cancel'] = array(
      '#type' => 'link',
      '#title' => t('Cancel'),
      '#href' => "feed/{$feed->fid}/feed-items",
    );
    
    $form['#theme'] = 'confirm_form';
    
    return $form;
  }
  
  $form_state['#feed'] = feed_defaults($feed);
  
  if (empty($form_state['#pager'])) {
    $form_state['#pager'] = new FeedItemPager(20);
    $form_state['#pager']->getFeedItemQuery()->condition('fi.fid', $feed->fid);
  }
  
  $form['filter'] = array(
    '#type' => 'feed_item_filter',
    '#terms' => &$session['filter_terms'],
    '#keywords' => &$session['filter_keywords'],
    '#pager' => &$form_state['#pager'],
    '#ajax' => array(
      'callback' => 'feed_item_filter_ajax',
      'wrapper' => 'feed-item-select-filtered',
      'method' => 'replace',
    ),
  );
  
  $form['select'] = array('#type' => 'container', '#tree' => TRUE);
  $form['select']['feed_items'] = array(
    '#type' => 'feed_item_select',
    '#id' => 'feed-item-select-filtered',
    '#pager' => &$form_state['#pager'],
    '#links' => array(
      'edit' => array(
        '#title' => t('Edit'),
        '#href' => 'feed/%fid%/feed-items/%fiid%',
        '#options' => array('attributes' => array('title' => t('Edit feed item'))),
      ),
      'delete' => array(
        '#title' => t('Delete'),
        '#href' => 'feed/%fid%/feed-items/%fiid%/delete',
        '#options' => array('attributes' => array('title' => t('Delete feed item'))),
      ),
    ),
  );
  
  $form['select']['operation'] = array(
    '#type' => 'container', 
    '#id' => 'feed-item-operation-action-container',
    '#tree' => TRUE,
  );
  $form['select']['operation']['action'] = array(
    '#type' => 'select',
    '#title' => t('Operation'),
    '#empty_option' => '- Select action -',
    '#id' => 'feed-item-operation-action',
    '#options' => array(
      'publish' => t('Publish selected items'),
      'unpublish' => t('Unpublish selected items'),
      'tag' => t('Tag selected items'),
      'untag' => t('Untag selected items'),
      'delete' => t('Delete selected items'),
    ),
  );
  $form['select']['operation']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#attributes' => array('class' => array('feed-item-operation-submit')),
  );
  $form['select']['operation']['clear'] = array('#markup' => '<br style="clear: both;"/>');
  $form['select']['add_terms'] = array(
    '#type' => 'feed_category_term_autocomplete',
    '#title' => t('Add Tags'),
    '#remove_text' => 'remove',
    '#states' => array(
      'visible' => array(
        '#feed-item-operation-action' => array('value' => 'tag'),
      )
    )
  );
  $form['select']['remove_terms'] = array(
    '#type' => 'feed_category_term_autocomplete',
    '#title' => t('Remove Tags'),
    '#remove_text' => 'remove',
    '#states' => array(
      'visible' => array(
        '#feed-item-operation-action' => array('value' => 'untag'),
      )
    )
  );
  
  $form['#attributes'] = array('class' => array('feed-form'));
  return $form;
}

/**
 * Ajax callback for feed_item_filter element.
 */
function feed_item_filter_ajax($form, &$form_state) {
  return $form['select']['feed_items'];
}

/**
 * Feed item category term form submit handler;
 */
function feed_item_overview_form_submit($form, &$form_state) {
  $feed = $form_state['#feed'];
  
  if (!empty($form_state['values']['delete_confirm']) && !empty($form_state['values']['delete_items'])) {
    $deleted_titles = array();
    $error_titles = array();
    
    foreach (array_keys($form_state['values']['delete_items']) as $fiid) {
      if (!$feed_item = feed_item_load($fiid)) {
        continue;
      }
      $title = feed_item_field_values_tagname_format($feed_item->fiid, 'title', 'text');
      if ($feed_item->fid == $feed->fid && feed_item_delete($feed_item)) {
        $deleted_titles[] = $title;
      }
      else {
        $error_titles[] = $title;
      }
    }
    if (!empty($deleted_titles)) {
      drupal_set_message(t('Successfully deleted feed items: @items', array('@items' => implode(', ', $deleted_titles), 'status')));
    }
    if (!empty($error_titles)) {
      drupal_set_message(t('Error deleting feed items: @items', array('@items' => implode(', ', $error_titles), 'error')));
    }
  }
  else {
    switch ($form_state['values']['op']) {
      case $form['filter']['filter']['submit']['#value']:
        $form_state['rebuild'] = TRUE;
      break;
      
      case $form['select']['operation']['submit']['#value']:
        $feed_items = $form['select']['feed_items']['#feed_item_selected'];
        
        if (!empty($feed_items)) {
          switch ($form_state['values']['select']['operation']['action']) {
            case 'publish':
              $status = feed_item_publish($feed_items);
            
              $count_feed_item = count($feed_items);
              $count_status = count(array_filter($status));
            
              if ($count_status > 0) {
                drupal_set_message(t('Successfully published %count feed item(s)', array('%count' => $count_status)), 'status');
              }
              if ($count_feed_item > $count_status) {
                drupal_set_message(t('Error publishing %count feed item(s)', array('%count' => $count_feed_item - $count_status)), 'error');
              }
            break;
            
            case 'unpublish':
              $status = feed_item_unpublish($feed_items);
            
              $count_feed_item = count($feed_items);
              $count_status = count(array_filter($status));
              
              if ($count_status > 0) {
                drupal_set_message(t('Successfully unpublished %count feed item(s)', array('%count' => $count_status)), 'status');
              }
              if ($count_feed_item > $count_status) {
                drupal_set_message(t('Error unpublishing %count feed item(s)', array('%count' => $count_feed_item - $count_status)), 'error');
              }
            break;
            
            case 'tag':
              if (!empty($form_state['values']['select']['add_terms'])) {
                $autocomplete_id = $form['select']['add_terms']['#id'];
                
                if (!empty($form_state[$autocomplete_id]['terms']) && is_array($form_state[$autocomplete_id]['terms'])) {
                  $feed_item_category_terms = array();
                  
                  $tag_status = array('success' => array(), 'errors' => array());
                  foreach ($form_state[$autocomplete_id]['terms'] as $feed_category_term) {
                    if (!isset($feed_category_term->fctid)) {
                      $feed_category = feed_category_load($feed_category_term->fcid);
                      
                      if (!feed_category_term_save($feed_category_term, $feed_category)) {
                        drupal_set_message(t('Error creating tag %category: %name', array(
                        	'%category' => ucwords($feed_category->name),
                          '%name' => $feed_category_term->name
                        )), 'error');
                        continue;
                      }
                    }
                    $feed_item_category_terms[$feed_category_term->fctid] = $feed_category_term;
                  }
                  
                  foreach ($feed_items as $feed_item) {
                    if (!feed_item_tag_category_terms($feed_item, $feed_item_category_terms)) {
                      $title = feed_field_values_tagname_format($feed_item->fid, 'title', 'text');
                      drupal_set_message(t('Error tagging feed item %title', array('%title' => $title)), 'error');
                    }
                  }
                }
              }
            break;
            
            case 'untag':
              if (!empty($form_state['values']['select']['remove_terms'])) {
                $autocomplete_id = $form['select']['remove_terms']['#id'];
                
                if (!empty($form_state[$autocomplete_id]['terms']) && is_array($form_state[$autocomplete_id]['terms'])) {
                  foreach ($feed_items as $feed_item) {
                    if (!feed_item_untag_category_terms($feed_item, $form_state[$autocomplete_id]['terms'])) {
                      $title = feed_field_values_tagname_format($feed_item->fid, 'title', 'text');
                      drupal_set_message(t('Error untagging feed item %title', array('%title' => $title)), 'error');
                    }
                  }
                }
              }
            break;
            
            case 'delete':
              $form_state['#delete'] = $feed_items;
              $form_state['rebuild'] = TRUE;
            break;
            
            default:
              form_error($form['select']['operation'], 'Selected action is not a valid operation.');
            break;
          }
        }
        else {
          form_error($form['select']['operation'], 'No items selected.');
        }
      break;
    }
  }
}

/**
 * Manage feed item form;
 */
function manage_feed_item_form($form, &$form_state, $feed = NULL) {
  $form_state['#feed'] = feed_defaults($feed);
  
  feed_item_field_attach($form, $form_state);
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Manage feed item form submit handler;
 */
function manage_feed_item_form_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  
  feed_item_field_attach_submit($form, $form_state);
}

/**
 * Delete feed item form;
 */
function feed_item_delete_form($form, &$form_state, $feed, $feed_item) {
  $form_state['#feed'] = feed_defaults($feed);
  $form_state['#feed_item'] = feed_item_defaults($feed_item);

  $form['#attributes'] = array('class' => array('feed-form'));
  $title = feed_item_field_values_tagname_format($feed_item->fiid, 'title', 'text');
  
  return confirm_form(
    $form,
    t('Delete feed item?'),
    "feed/{$feed->fid}/feed-items",
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete a feed item with title ":title". This action cannot be undone.', array(':title' => $title)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed item form submit handler;
 */
function feed_item_delete_form_submit(&$form, &$form_state) {
  $feed = $form_state['#feed'];
  $feed_item = $form_state['#feed_item'];
  
  $title = feed_item_field_values_tagname_format($feed_item->fiid, 'title', 'text');
  if (feed_item_delete($feed_item)) {
    drupal_set_message(t('Feed item with title ":title" was deleted.', array(':title' => $title)));
  } 
  else {
    drupal_set_message(t('Could not delete feed item with title ":title".', array(':title' => $title)), 'error');
  }
  $form_state['redirect'] = "feed/{$feed->fid}/feed-items";
}

/**
 * Feed item field settings form;
 */
function feed_item_field_settings_form($form, &$form_state, $feed = NULL, $feed_field = NULL) {
  if (empty($feed_field->is_feed_item_field)) {
    drupal_goto("feed/{$feed->fid}/fields/{$feed_field->ffid}"); exit();
  }
  
  $form_state['#feed'] = feed_defaults($feed);
  $form_state['#feed_field'] = feed_field_defaults($feed_field);
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => $form_state['#feed_field']->name,
    '#description' => t('The administrative label for this feed field'),
    '#required' => TRUE,
  );
  $form['tag'] = array(
    '#type' => 'textfield',
    '#title' => t('Tag name.'),
    '#default_value' => $form_state['#feed_field']->machine_name,
    '#description' => t('The tag name for this feed field'),
    '#required' => TRUE,
  );
  $form['namespace'] = array(
    '#type' => 'textfield',
    '#title' => t('Tag namespace.'),
    '#default_value' => $form_state['#feed_field']->machine_name,
    '#description' => t('The tag namespace for this feed field'),
    '#required' => TRUE,
  );
  
  // Run handlers to attach feed item field settings
  feed_item_field_settings_attach($form, $form_state);
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed field settings'),
    '#weight' => 40,
  );
  
  return $form;
}

/**
 * Feed item field settings form submit handler;
 */
function feed_item_field_settings_form_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  
  $feed_field =& $form_state['#feed_field'];
  $feed_field->name = trim($form_state['values']['name']);
  $feed_field->tag = trim($form_state['values']['tag']);
  $feed_field->namespace = $form_state['values']['namespace'];
  
  // Run handlers to apply feed field settings
  feed_item_field_settings_attach_submit($form, $form_state);

  $feed_fields = array();
  $feed_fields[] =& $feed_field;
  
  $status = feed_fields_save($feed_fields, $feed, TRUE);
  
  if ($status !== FALSE) {
    
    if ($status == SAVED_UPDATED) {
      drupal_set_message(t('The feed item field %name has been updated.', array('%name' => $feed_field->name)));
    }
    elseif ($status == SAVED_NEW) {
      drupal_set_message(t('The feed item field %name has been added.', array('%name' => $feed_field->name)));
    }
  }
}

/**
 * Feed item field widgets form;
 */
function feed_item_form($form, &$form_state, $feed = NULL, $feed_item = NULL) {
  if (!isset($form_state['#feed'])) {
    $form_state['#feed'] = feed_defaults($feed);
  }
  
  if (!isset($form_state['#feed_items'])) {
    $feed_items = array();
    if (is_array($feed_item)) {
      foreach ($feed_item as $item) {
        $feed_items[] = feed_item_defaults($item);
      }
    }
    else {
      $feed_items[] = feed_item_defaults($feed_item);
    }
    $form_state['#feed_items'] = $feed_items;
  }
  
  if (!isset($form_state['#feed_item_fields'])) {
    $form_state['#feed_item_fields'] = feed_load_fields($form_state['#feed'], TRUE);
  }
  uasort($form_state['#feed_item_fields'], 'feed_fields_sort');
  
  $form['feed_items'] = array('#tree' => TRUE);
  foreach ($form_state['#feed_items'] as $index => &$feed_item) {
    $form['feed_items'][$index] = array(
      '#tree' => TRUE,
      '#type' => 'feed_fieldset',
      '#title' => t('Feed Item'),
      '#attributes' => array('class' => array('feed-item-fieldset')),
    );
    
    // Run handlers to attach feed widgets
    $form['feed_items'][$index]['feed_item_fields_required'] = array('#type' => '#container');
    $form['feed_items'][$index]['category'] = array(
      '#type' => 'feed_category_term_autocomplete',
      '#title' => t('Category Tags'),
      '#remove_text' => 'remove',
      '#default_value' => array(
        'feed_category_terms' => isset($feed_item->fiid) ? feed_item_category_terms($feed_item) : array()
      ),
    );
    $form['feed_items'][$index]['feed_item_fields'] = array('#type' => '#tree');
    
    foreach ($form_state['#feed_item_fields'] as $ffid => &$feed_item_field) {
      if (empty($feed_item_field->is_required)) {
        continue;
      }
      $key = !empty($feed_item_field->is_required) ? 'feed_item_fields_required' : 'feed_item_fields_optional';
      $feed_item_field_values = !empty($feed_item->fiid) ? feed_item_field_values_load($feed_item, $feed_item_field) : array();
      
      $form['feed_items'][$index][$key][$ffid] = array(
        '#type' => 'feed_item_field_widget',
        '#feed_item_field' => $feed_item_field,
        '#feed_item_field_values' => $feed_item_field_values,
      );
    }
  }
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t(count($form_state['#feed_items']) > 1 ? 'Save feed items' : 'Save feed item'),
    '#weight' => 40,
    '#attributes' => array('class' => array('feed-item-save-submit', 'feed-submit')),
  );
  
  if (!isset($feed_item->fiid)) {
    $form['actions']['add'] = array(
      '#type' => 'submit',
      '#value' => 'Add another feed item',
      '#weight' => 40,
      '#attributes' => array('class' => array('feed-item-add-submit', 'feed-submit')),
    );
  }
  
  return $form;
}

/**
 * Feed item field widgets form submit handler;
 */
function feed_item_form_submit($form, &$form_state) {
  $feed =& $form_state['#feed'];
  $feed_items =& $form_state['#feed_items'];
  $feed_item_fields =& $form_state['#feed_item_fields'];
  $feed_item_field_values =& $form_state['#feed_item_field_values'];
  
  if (isset($form['actions']['add']['#value']) && $form_state['values']['op'] == $form['actions']['add']['#value']) {
    $feed_items[] = feed_item_defaults();
    $form_state['rebuild'] = TRUE;
  }
  else {
    foreach ($form_state['values']['feed_items'] as $index => $values) {
      if (!isset($feed_items[$index])) {
        continue;
      }
      $feed_item =& $feed_items[$index];
      $feed_item->fid = $feed->fid;
      
      $status = feed_item_save($feed_item, $feed);
      if ($status !== FALSE) {
        foreach (array('feed_item_fields_required', 'feed_item_fields') as $widget_key) {
          foreach (element_children($form['feed_items'][$index][$widget_key]) as $key) {
            $feed_item_field_widget  = $form['feed_items'][$index][$widget_key][$key];
            
            if (isset($feed_item_field_widget['#feed_item_field_values'], $feed_item_field_widget['#feed_item_field'])) {
              if (!feed_item_field_values_save($feed_item_field_widget['#feed_item_field_values'], $feed_item, $feed_item_field_widget['#feed_item_field'])) {
                drupal_set_message(t('Error saving value for field %field.', array('%field' => $feed_item_field_widget['#feed_item_field']->name)), 'error');
              }
            }
          }
        }
        
        if (!empty($form['feed_items'][$index]['category']['#id'])) {
          $autocomplete_id = $form['feed_items'][$index]['category']['#id'];
          if (!empty($form_state[$autocomplete_id]['terms']) && is_array($form_state[$autocomplete_id]['terms'])) {
            feed_item_untag_all_category_terms($feed_item);
            
            foreach ($form_state[$autocomplete_id]['terms'] as $feed_category_term) {
              if (!isset($feed_category_term->fctid)) {
                $feed_category = feed_category_load($feed_category_term->fcid);
                
                if (!feed_category_term_save($feed_category_term, $feed_category)) {
                  form_error($form['feed_items'][$index]['category'], t('Unable to save term %term', array('%term' => $feed_category_term->name)));
                  continue;
                }
              }
              if (empty($feed_category_term->fctid) || !feed_item_tag_category_term($feed_item, $feed_category_term)) {
                form_error($form['feed_items'][$index]['category'], t('Unable to tag feed item with term %term', array('%term' => $feed_category_term->name)));
              }
            }
          }
        }
        
        $title = !empty($feed_item->fiid) ? feed_item_field_values_tagname_format($feed_item->fiid, 'title', 'text') : '';
        
        if ($status == SAVED_UPDATED) {
          drupal_set_message(t('The feed item "%title" has been updated.', array('%title' => $title)));
        }
        elseif ($status == SAVED_NEW) {
          drupal_set_message(t('The feed item"%title" has been added.', array('%title' => $title)));
        }
        else {
          drupal_set_message(t('The feed item "%title" could not be saved.', array('%title' => $title)), 'error');
        }
      }
    }
    $form_state['redirect'] = "feed/{$feed->fid}/feed-items";
  }
}