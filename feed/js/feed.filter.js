(function($) {
  Drupal.feed = $.extend(Drupal.feed || {}, {
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
    }
  });
  
  $(function() {
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
      
      filter.find('.feed-filter-category .form-type-checkboxes').each(function() {
        var trigger = $(this).children('label');
        var target = $(this).children('.feed-filter-category-terms');

        trigger.click(function() {
          if (target.is(':visible')) {
            trigger.find('.feed-category-toggle').text('+');
            target.slideUp();
          }
          else {
            trigger.find('.feed-category-toggle').text('-');
            target.slideDown();
          }
        }).click();
      });
    }
  });
})(jQuery);
