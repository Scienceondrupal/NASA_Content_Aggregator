(function($) {
  
  Drupal.behaviors.feed = {
    attach: function (context, settings) {
      $('.feed-fieldset-collapsible').each(function() {
        var fieldset = $(this);
        var trigger = fieldset.find('.feed-fieldset-legend-wrapper');
        var target = fieldset.find('.feed-fieldset-wrapper');
        
        trigger.click(function() {
          if (fieldset.hasClass('feed-fieldset-collapsible-collapsed')) {
            target.slideDown(function() {
              fieldset.removeClass('feed-fieldset-collapsible-collapsed');
            });
          }
          else {
            target.slideUp(function() {
              fieldset.addClass('feed-fieldset-collapsible-collapsed');
            });
          }
        });
      });
      
      $('.feed-widget-show-snippet').each(function() {
        var target;
        if ($(this).attr('data-widget-target')) {
          target = $('.' + $(this).attr('data-widget-target'));
        }
        $(this).click(function(e) {
          e.preventDefault();
          if (target.length) {
            target.toggle();
          }
        });
      });
      
      $('[id="feed-item-select"]', context).once(function () {
        var $select = $(this);
        $select.find('input.feed-item-select-check-all').click(function() {
          $select.find('input:checkbox').not(this).attr('checked', $(this).attr('checked'));
        });
      });
      
      var feed_item_detailed = $('.feed-item-detailed');
      if (feed_item_detailed.length) {
        feed_item_detailed.each(function() {
          var feed_item_detail = $(this);
          feed_item_detail.click(function(e) {
            e.preventDefault();
            
            if (e.isDefaultPrevented()) {
              Drupal.feed.feedItemDetailed(feed_item_detail.attr('data-feed-item'));
            }
          });
        })
      }
    }
  };
  
  Drupal.feed = $.extend(Drupal.feed || {}, {
    feedItemDetailed: function(id) {
      var overlay = $('<div class="feed-item-detailed-overlay"></div>').appendTo('body');
      var wrapper = $('<div class="feed-item-detailed-wrapper"></div>').appendTo('body');
      var close = $('<div class="feed-item-detailed-close"></div>').appendTo(wrapper);
      
      close.add(overlay).click(function(e) {
        overlay.remove();
        wrapper.remove();
      });
      
      $.ajax({
        'url': '/feed-item/' + id + '/format/json',
        'dataType': 'jsonp',
        'data': {
          'show': {
            'items': ['title', 'description'],
          },
          'format': {
            'items': {
              'title': 'text',
              'link': 'text',
              'description': 'text'
            }
          }
        },
        'success': function(data) {
          if ($.isPlainObject(data)) {
            $.each(data, function(name, values) {
              var title = name.replace('/\\s/', '_').replace(/[^a-z0-9-_]/i, '');
              var field = $('<div class="feed-item-detailed-field"></div>');
              
              field.append('<div class="feed-item-detailed-field-title">' + title + '</div>');
              
              $.each(values, function(i, value) {
                field.append($('<div class="feed-item-detailed-field-value"></div>').html(value));
              });
              wrapper.append(field);
            });
          }
        },
        'error': function(xhr, status, error) {
          wrapper.remove();
          overlay.remove();
        }
      });
    }
  });
  
})(jQuery);
