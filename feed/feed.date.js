(function($){
  Drupal.behaviors.FeedDate = {
    attach: function(context, settings) {
      $('.feed-date-widget-date').datepicker();
    }
  };
})(jQuery);