<?php 

/**
 * Returns HTML for a feed fieldset form element and its children.
 *
 * @see theme_fieldset().
 */
function theme_feed_fieldset($variables) {
  $element = $variables['element'];
  element_set_attributes($element, array('id'));
  _form_set_class($element, array('feed-fieldset', 'form-wrapper'));

  $output = '<div' . drupal_attributes($element['#attributes']) . '>';
  if (!empty($element['#title'])) {
    // Always wrap fieldset legends in a SPAN for CSS positioning.
    $output .= '<div class="feed-fieldset-legend-wrapper"><span class="feed-fieldset-legend">' . $element['#title'] . '</span></div>';
  }
  $output .= '<div class="feed-fieldset-wrapper">';
  if (!empty($element['#description'])) {
    $output .= '<div class="feed-fieldset-description">' . $element['#description'] . '</div>';
  }
  $output .= $element['#children'];
  if (isset($element['#value'])) {
    $output .= $element['#value'];
  }
  $output .= '</div>';
  $output .= "</div>\n";
  return $output;
}

/**
 * Returns HTML for a feed table form element and its children.
 */
function theme_feed_table($variables) {
  $element = $variables['element'];
  
  $element['#rows'] = array();
  foreach (element_children($element, TRUE) as $row_index) {
    $cells = $element[$row_index];
    $row = (isset($cells['#row']) && is_array($cells['#row'])) ? $cells['#row'] : array();
    unset($cells['#row']);
    $row['data'] = array();
    
    foreach (element_children($cells, TRUE) as $cell_index) {
      $data = $cells[$cell_index];
      $cell = (isset($data['#cell']) && is_array($data['#cell'])) ? $data['#cell'] : array();
      unset($data['#cell']);
      $cell['data'] = $data;
      $row['data'][] = $cell;
    }
    
    $element['#rows'][] = $row;
  }
  $element['#theme'] = 'table';

  return theme('feed_block', array(
    'attributes' => array('id' => $element['#id']),
    'title' => !empty($element['#title']) ? $element['#title'] : '',
    'content' => $element,
  ));
}

/**
 * Returns HTML for a feed category term autocomplete element
 */
function theme_feed_category_term_autocomplete($variables) {
  $element = $variables['element'];
  $attributes = array();
  if (isset($element['#id'])) {
    $attributes['id'] = $element['#id'];
  }
  $attributes['class'][] = 'feed-category-term-autocomplete';
  if (!empty($element['#attributes']['class'])) {
    $attributes['class'] = array_merge($attributes['class'], $element['#attributes']['class']);
  }
  return '<div' . drupal_attributes($attributes) . '>' . (!empty($element['#children']) ? $element['#children'] : '') . '</div>';
}

/**
 * Returns HTML for a feed category term autocomplete item element
 * 
 * @todo delete?
 */
function theme_feed_category_term_autocomplete_item($variables) {
  $element = $variables['element'] + array(
    '#name' => '',
    '#value' => '',
    '#term' => '',
    '#category' => '',
    '#remove_text' => '',
  );
  
  $output = '<div class="feed-category-term-autocomplete-term">';
  $output .= '<a href="#" class="feed-category-term-autocomplete-remove">' . $element['#remove_text'] . '</a>';
  $output .= '<input class="feed-category-term-autocomplete-item" type="hidden" name="' . $element['#name'] . '" value="' . $element['#value'] . '" />';
  $output .= '<span class="feed-category-term-autocomplete-title">' . t('%category: %term', array('%category' => $element['#category'], '%term' => $element['#term'])) . '</span>';
  $output .= '</div>';
  
  return $output;
}