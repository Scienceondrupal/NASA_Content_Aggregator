(function($) {
  
  Drupal.behaviors.feed = {
    attach: function (context, settings) {
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
      
      var filter = $('#feed-item-filter');
      var select = $('.feed-item-select-container');
      if (filter.length) {
        Drupal.feed.filterScroller(filter, select);
        
        var filterDelay, filterSubmit = $('.feed-filter-ajax-submit');
        if (filterSubmit.length) {
          filter.find('.feed-filter-ajax-trigger:input').change(function() {
            clearTimeout(filterDelay);
            filterDelay = setTimeout(function() {
              filterSubmit.mousedown();
            }, 700);
          });
        }
      }
      
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
  
  Drupal.feed = {
    filterScroller: function(filterElem, selectElem) {
      var filter = $(filterElem);
      var select = $(selectElem);
      
      var wrapper = $('<div />', {
        'class': 'feed-item-filter-wrapper',
        'height': filter.outerHeight(),
        'width': filter.outerWidth(),
        'css': {'float': 'left'}
      });
      var toolbar = Drupal.toolbar && Drupal.toolbar.height ? Drupal.toolbar.height() : 0;
      var margin = parseFloat(filter.css('margin-top')) || 0;
      var offsetMin = filter.offset().top - toolbar - margin;
      var offsetMax = select.offset().top + select.outerHeight() - filter.outerHeight();

      if (select.height() > filter.height()) {
        $(window).bind('scroll.feed-filter', function(e) {
          var scrollTop = $(this).scrollTop();
  
          if (scrollTop < offsetMin) {
            filter.removeClass('feed-filter-floating').css({'position': 'relative', 'top': 0, 'bottom': 'auto'});
          }
          else if (scrollTop > offsetMax) {
            filter.removeClass('feed-filter-floating').css({'position': 'absolute', 'bottom': 0, 'top': 'auto'});
          }
          else if (scrollTop > offsetMin) {
            filter.addClass('feed-filter-floating').css({'position': 'fixed', 'top': toolbar, 'bottom': 'auto'});
          }
        });
      }
    },
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
  };
  
})(jQuery);
