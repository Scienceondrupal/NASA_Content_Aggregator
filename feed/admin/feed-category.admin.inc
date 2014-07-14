<?php

/**
 * Menu callback; Shows the feed category list.
 */
function feed_category_overview() {
  $feed_categories = feed_category_load_all_user();
  
  $header = array(t('Name'), t('Description'), array('data' => t('Operations'), 'colspan' => '4'));
  $rows = array();

  foreach ($feed_categories as $feed_category) {
    $row = array();
    $row[] = array('data' => t($feed_category->name));
    $row[] = array('data' => t($feed_category->description));
    $row[] = array('data' => l(t('view terms'), "feed/categories/{$feed_category->fcid}/terms"));
    $row[] = array('data' => l(t('add new term'), "feed/categories/{$feed_category->fcid}/terms/add"));
    
    if (feed_category_access(FALSE, $feed_category)) {
      $row[] = array('data' => l(t('edit'), "feed/categories/{$feed_category->fcid}/edit"));
      $row[] = array('data' => l(t('delete'), "feed/categories/{$feed_category->fcid}/delete"));
    }
    else {
      $row[] = array('data' => '&nbsp;', 'colspan' => '2');
    }

    $rows[] = $row;
  }
  
  $build = array();
  $build['feed_links'] = array(
    '#theme' => 'links',
    '#attributes' => array('class' => array('feed-links')),
    '#links' => array(
      'add_feed' => array(
        'title' => 'Add Category',
        'href' => 'feed/categories/add',
        'attributes' => array(
          'class' => array('button'),
        ),
      ),
    ),
  );
  $build['feed_category_overview'] = array(
    '#theme' => 'feed_block',
    '#attributes' => array('id' => 'feed-category-overview'),
    '#title' => 'Feed Category List',
    '#content' => array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
    	'#empty' => t('No feed categories available. <a href="@link">Add feed category</a>.', array('@link' => url('feed/categories/add'))),
    ),
  );
  return $build;
}

/**
 * Feed category form; 
 */
function feed_category_form($form, &$form_state, $feed_category = NULL) {
  $form_state['#feed_category'] = feed_category_defaults($feed_category);
  
  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['#feed_category']->name) ? $form_state['#feed_category']->name : '',
    '#description' => t('The human-readable name of this feed category. This name must be unique.'),
    '#required' => TRUE,
    '#size' => 60,
  );
  $form['description'] = array(
    '#title' => t('Description'),
    '#type' => 'textfield',
    '#default_value' => isset($form_state['#feed_category']->description) ? $form_state['#feed_category']->description : '',
    '#description' => t('Describe this feed category.'),
  );
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed category'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Feed category form submit handler; 
 */
function feed_category_form_submit(&$form, &$form_state) {
  global $user;
  
  $feed_category =& $form_state['#feed_category'];
  $feed_category->name = trim($form_state['values']['name']);
  $feed_category->uid = $user->uid;
  $feed_category->description = trim($form_state['values']['description']);

  $status = feed_category_save($feed_category);
  
  if ($status == SAVED_UPDATED) {
    drupal_set_message(t('The feed category %name has been updated.', array('%name' => $feed_category->name)));
  }
  elseif ($status == SAVED_NEW) {
    drupal_set_message(t('The feed category %name has been added.', array('%name' => $feed_category->name)));
  }
  $form_state['redirect'] = 'feed/categories';
}

/**
 * Delete feed category form;
 */
function feed_category_delete_form($form, &$form_state, $feed_category) {
  $form_state['#feed_category'] = feed_category_defaults($feed_category);

  $form['#attributes'] = array('class' => array('feed-form'));
  
  return confirm_form(
    $form,
    t('Delete feed category?'),
    'feed/categories',
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete feed category ":name". This action cannot be undone and all feed terms for this category will be lost', array(':name' => $feed_category->name)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed category form submit handler;
 */
function feed_category_delete_form_submit(&$form, &$form_state) {
  $feed_category = $form_state['#feed_category'];
  
  if (feed_category_delete($feed_category)) {
    drupal_set_message(t('Feed category :name was deleted.', array(':name' => $feed_category->name)), 'status');
  } 
  else {
    drupal_set_message(t('Could not delete feed category :name.', array(':name' => $feed_category->name)), 'error');
  }
  $form_state['redirect'] = 'feed/categories';
}

/**
 * Manage feed category term form;
 */
function manage_feed_category_term_form($form, &$form_state, $feed_category = NULL) {
  $form_state['#feed_category'] = feed_category_defaults($feed_category);
  $form_state['#feed_category_terms'] = feed_category_term_load_all($form_state['#feed_category']);
  
  $form['feed_category_term'] = array();
  $form['feed_category_term']['#type'] = 'feed_category_term';
  $form['feed_category_term']['#tree'] = TRUE;
  
  foreach ($form_state['#feed_category_terms'] as $fctid => $feed_category_term) {
    if (!isset($form['feed_category_term']['terms'])) {
      $form['feed_category_term']['terms'] = array('#tree' => TRUE);
    }
    
    $form['feed_category_term']['terms'][$feed_category_term->fctid] = array('#tree' => TRUE, '#weight' => $feed_category_term->weight);
    $form['feed_category_term']['terms'][$feed_category_term->fctid]['id'] = array(
      '#type' => 'hidden',
      '#value' => !empty($feed_category_term->fctid) ? $feed_category_term->fctid : NULL,
      '#attributes' => array(
        'class' => array('feed-category-term-id'),
      ),
    );
    $form['feed_category_term']['terms'][$feed_category_term->fctid]['name'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="feed-category-term-name">' . (!empty($feed_category_term->name) ? $feed_category_term->name : '') . '</div>',
      '#attributes' => array('class' => array('feed-category-term-name')),
      '#title_display' => 'invisible',
      '#title' => t('Name'),
    );
    $form['feed_category_term']['terms'][$feed_category_term->fctid]['weight'] = array(
      '#type' => 'weight',
      '#delta' => 50,
      '#default_value' => is_numeric($feed_category_term->weight) ? $feed_category_term->weight : 0,
      '#title_display' => 'invisible',
      '#title' => t('Weight for feed category term'),
      '#attributes' => array('class' => array('feed-category-term-weight')),
    );
    
    if (feed_category_term_access($feed_category_term)) {
      $form['feed_category_term']['terms'][$feed_category_term->fctid]['edit'] = array(
        '#type' => 'link',
        '#title' => t('Edit Term'),
        '#title_display' => 'invisible',
        '#href' => 'feed/categories/' . $form_state['#feed_category']->fcid . '/terms/' . $feed_category_term->fctid . '/edit',
        '#options' => array('attributes' => array('title' => t('Edit term.'))),
      );
      $form['feed_category_term']['terms'][$feed_category_term->fctid]['delete'] = array(
        '#type' => 'link',
        '#title' => t('Delete Term'),
        '#title_display' => 'invisible',
        '#href' => 'feed/categories/' . $form_state['#feed_category']->fcid . '/terms/' . $feed_category_term->fctid . '/delete',
        '#options' => array('attributes' => array('title' => t('Delete term.'))),
      );
    }
  }
  
  // Add new term
  $form['feed_category_term']['add-term'] = array('#tree' => TRUE);
  $form['feed_category_term']['add-term']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('New term'),
    '#title_display' => 'invisible',
    '#size' => 15,
    '#description' => t('New term'),
    '#attributes' => array(
      'class' => array('feed-category-term-add-name'),
    ),
  );
  $form['feed_category_term']['add-term']['weight'] = array(
    '#type' => 'weight',
    '#delta' => 50,
    '#default_value' => 0,
    '#title_display' => 'invisible',
    '#title' => t('Weight for new term'),
    '#description' => t('Weight'),
    '#attributes' => array(
      'class' => array('feed-category-term-add-weight'),
    ),
  );
  $form['feed_category_term']['add-term']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add Term'),
    '#attributes' => array(
      'class' => array('feed-category-term-add-submit'),
    ),
  );
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed category'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Manage feed category term form submit handler;
 */
function manage_feed_category_term_form_submit(&$form, &$form_state) {
  global $user;
  
  $feed_category =& $form_state['#feed_category'];
  
  if ($form_state['values']['op'] == $form['feed_category_term']['add-term']['add']['#value']) {
    $feed_category_term_values = $form_state['values']['feed_category_term']['add-term'];
    
    if (trim($feed_category_term_values['name']) !== '') {
      $feed_category_term = feed_category_term_defaults();
      $feed_category_term->name = trim($feed_category_term_values['name']);
      $feed_category_term->weight = $feed_category_term_values['weight'];
      
      if (!isset($feed_category_term->uid)) {
        $feed_category_term->uid = $user->uid;
      }
      if (!feed_category_term_save($feed_category_term, $feed_category)) {
        form_error($form['feed_category_term']['add-term']['name'], t('Error saving category term.'));
      }
      else {
        $form_state['redirect'] = 'feed/categories/' . $feed_category->fcid;
      }
    }
    else {
      form_error($form['feed_category_term']['add-term']['name'], t('Please enter a term.'));
    }
  }
  else {
    $feed_category_terms =& $form_state['#feed_category_terms'];
    
    $feed_category_terms_stack = array();
    
    if (!empty($feed_category_terms)) {
      foreach ($feed_category_terms as $feed_category_term) {
        if (isset($form_state['values']['feed_category_term']['terms'][$feed_category_term->fctid])) {
          $values = $form_state['values']['feed_category_term']['terms'][$feed_category_term->fctid];
          
          if (isset($values['weight']) && is_numeric($values['weight'])) {
            $feed_category_term->weight = $values['weight'];
          }
          $feed_category_terms_stack[$feed_category_term->fctid] = $feed_category_term;
        }
      }
    }
    if (!empty($feed_category_terms_stack)) {
      if ($status = feed_category_terms_save($feed_category_terms_stack, $feed_category)) {
        drupal_set_message(t('The feed category %name has been updated.', array('%name' => $feed_category->name)));
      }
    }
  }
}

/**
 * Feed category term form;
 */
function feed_category_term_form($form, &$form_state, $feed_category = NULl, $feed_category_term = NULL) {
  $form_state['#feed_category'] = feed_category_defaults($feed_category);
  $form_state['#feed_category_term'] = feed_category_term_defaults($feed_category_term);
  
  if (!empty($feed_category->name)) {
	  $form['category'] = array(
	    '#type' => 'markup',
	    '#markup' => '<h2>' . t('Term in Category: %category', array('%category' => $feed_category->name)) . '</h2>',
	  );
  }
	
  $form['name'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => isset($feed_category_term->name) ? $feed_category_term->name : '',
    '#description' => t('The name of this feed category term.'),
    '#required' => TRUE,
    '#size' => 60,
  );
  
  $form['#attributes'] = array('class' => array('feed-form'));
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save feed category term'),
    '#weight' => 40,
  );
  return $form;
}

/**
 * Feed category term form submit handler;
 */
function feed_category_term_form_submit(&$form, &$form_state) {
  global $user;
  
  $feed_category =& $form_state['#feed_category'];
  $feed_category_term =& $form_state['#feed_category_term'];
  
  $feed_category_term->name = trim($form_state['values']['name']);
  
  if (!isset($feed_category_term->uid)) {
    $feed_category_term->uid = $user->uid;
  }

  $status = feed_category_term_save($feed_category_term, $feed_category);
  
  if ($status == SAVED_UPDATED) {
    drupal_set_message(t('The feed category term %name has been updated.', array('%name' => $feed_category_term->name)));
  }
  elseif ($status == SAVED_NEW) {
    drupal_set_message(t('The feed category term %name has been added.', array('%name' => $feed_category_term->name)));
  }
  $form_state['redirect'] = 'feed/categories/' . $feed_category->fcid;
}

/**
 * Delete feed category term form;
 */
function feed_category_term_delete_form($form, &$form_state, $feed_category, $feed_category_term) {
  $form_state['#feed_category'] = feed_category_defaults($feed_category);
  $form_state['#feed_category_term'] = feed_category_term_defaults($feed_category_term);

  $form['#attributes'] = array('class' => array('feed-form'));
  
  return confirm_form(
    $form,
    t('Delete feed category term?'),
    "feed/categories/{$feed_category->fcid}",
    '<div class="messages warning"><span class="feed-warning">Warning:</span> ' . 
    t('You are about to delete feed category term ":name". This action cannot be undone', array(':name' => $feed_category_term->name)) . '</div>',
    t('Delete'),  t('Cancel')
  );
}

/**
 * Delete feed category term form submit handler;
 */
function feed_category_term_delete_form_submit(&$form, &$form_state) {
  $feed_category = $form_state['#feed_category'];
  $feed_category_term = $form_state['#feed_category_term'];
  
  if (feed_category_term_delete($feed_category_term)) {
    drupal_set_message(t('Feed category term :name was deleted.', array(':name' => $feed_category_term->name)));
  } 
  else {
    drupal_set_message(t('Could not delete feed category term :name.', array(':name' => $feed_category_term->name)), 'error');
  }
  $form_state['redirect'] = "feed/categories/{$feed_category->fcid}";
}