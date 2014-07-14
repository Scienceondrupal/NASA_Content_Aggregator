(function($){
  var FeedAutocomplete = function(input, url, options) {
    var o = $.extend(true, {
      'callbacks': {
        'select': null,
        'hide': null,
        'show': null,
        'previous': null,
        'next': null,
        'highlight': null,
        'unhighlight': null
      },
      'limit': 10,
      'search': {},
      'delay': 300
    }, options || {});
    
    var instance = this;
    
    this.active = false;
    this.input = $(input);
    this.term = '';
    this.selected = null;
    this.searcher;
    this.selection = $('<div />', {
      'class': 'feed-ac-selection'
    });
    
    this.cacher = (function() {
      var cache = {};
      var keys = [];
      var limit = o.limit;
        
      this.getkeys = function() {
        return keys;
      }
      this.has = function(key) {
        return key in cache;
      }
    
      this.set = function(key, value) {
        if (keys.length >= limit) {
          var last = keys.shift();
          delete cache[last];
        }
        if ($.inArray(key, keys) < 0) {
          keys.push(key);
        }
        cache[key] = value;
      }
      
      this.get = function(key) {
        return key && cache[key] ? cache[key] : null;
      }
      
      this.last = function() {
        return this.get(keys.slice(-1).pop());
      }
      
      this.first = function() {
        return this.get(keys.slice(0, 1).unshift());
      }
      return this;
    })();
    
    var query = $.extend(true, {
      'url': url,
      'dataType': 'json',
      'type': 'get',
      'error': function(xhr, status, error) {
        console.log(error);
      },
      'success': function(data){
        instance.selection.empty();
        
        if ($.isPlainObject(data)) {
          instance.cacher.set(instance.term, data);

          instance.selection
            .bind({
              'mouseenter': function() {
                instance.active = true;
              },
              'mouseleave': function() {
                instance.active = false;
              }
            })
            .append(instance.itemize(data))
            .find('.feed-ac-list-item')
            .bind({
              'click': function() {
                instance.select();
                instance.hide();
              },
              'mouseenter': function() {
                instance.highlight(this);
              },
              'mouseleave': function() {
                if (instance.selected && instance.selected[0] == this) {
                  instance.unhighlight();
                }
              }
            });
          instance.show();
        }
      }
    }, o.search || {});
    
    this.select = function() {
      var key = this.selected.find('.feed-ac-list-value').attr('data-feed-ac-key');
      var values = instance.cacher.last();
      var value = values['data'] && values['data'][key] || '';

      if ($.isFunction(o.callbacks.select)) {
        o.callbacks.select.apply(this, [input, o, key, value]);
      }
    };
    
    this.search = function(term) {
      instance.term = term;
      clearTimeout(instance.searcher);
      instance.searcher = setTimeout(function() {
        var config = $.extend(true, {'data': {'query': instance.term}}, query);

        if (instance.cacher.has(this.term)) {
          return $.isFunction(config.success) ? config.success.apply(instance, [instance.cacher.get(instance.term)]) : false;
        }
        else {
          $.ajax(config);
        }
      }, o.delay);
    }
    
    this.show = function() {
      var position = input.position();

      instance.selection.css({
        'position': 'absolute',
        'z-index': 1000,
        'top': parseInt(position.top + instance.input.outerHeight(), 10) + 'px',
        'left': parseInt(position.left, 10) + 'px',
        'width': instance.input.outerWidth() + 'px',
        'display': 'none'
      });
      instance.input.after(instance.selection);
      instance.selection.show();
      
      if ($.isFunction(o.callbacks.show)) {
        o.callbacks.show.apply(instance, [instance.input, o]);
      }
    };

    this.hide = function() {
      clearTimeout(instance.searcher);
      instance.selection.fadeOut('fast', function() {
        instance.unhighlight();
        instance.selection.detach();
        instance.selection.empty();
      });
      
      if ($.isFunction(o.callbacks.hide)) {
        o.callbacks.hide.apply(instance, [instance.input, o]);
      }
    };
    
    this.finalize = function() {
      if (instance.selected) {
        instance.select();
      }
      instance.hide();
    }
    
    this.itemize = function(items) {
      var container = $('<div />', {'class': 'feed-ac-list-container'});
      var list = $('<ul />', {'class': 'feed-ac-list'});
      
      if (items.title) {
        container.append($('<div />', {
          'class': 'feed-ac-list-title',
          'html': items.title
        }));
      }

      $.each(items.data || {}, function(key, value) {
        if ($.isPlainObject(value)) {
          if (value.data) {
            value = instance.itemize(value);
          }
          else if (value.label) {
            value = value.label;
          }
        }
        
        $('<li />', {'class': 'feed-ac-list-item'})
            .appendTo(list)
            .append($('<div />', {
              'data-feed-ac-key': key,
              'class': 'feed-ac-list-value',
              'html': value
            }));
      });
        
      return container.append(list);
    }
    
    this.previous = function() {
      if (instance.selected && instance.selected.prev().length) {
        instance.highlight(instance.selected.prev());
      }
      else {
        instance.highlight(instance.selection.find('.feed-ac-list-item:last'));
      }
      if ($.isFunction(o.callbacks.previous)) {
        o.callbacks.previous.apply(instance, [instance.input, o]);
      }
    };
    
    this.viewable = function() {
      if (instance.selected) {
        var scrollable = instance.selection.find('.feed-ac-list-container');
        var scroll_offset = scrollable.scrollTop();
        var elem_offset = instance.selected.position().top + scroll_offset;
        var elem_height = instance.selected.height();
        var container_height = instance.selection.innerHeight();
        
        if (elem_offset < scroll_offset || (elem_offset + elem_height) > (scroll_offset + container_height)) {
          console.log('scrollTo ' + (elem_offset + container_height / 2 - instance.selected.height() / 2));
          scrollable.scrollTop(elem_offset - container_height / 2 - instance.selected.height() / 2);
        }
      }
    }
    
    this.next = function() {
      if (instance.selected && instance.selected.next().length) {
        instance.highlight(instance.selected.next());
      }
      else {
        instance.highlight(instance.selection.find('.feed-ac-list-item:first'));
      }
      if ($.isFunction(o.callbacks.next)) {
        o.callbacks.next.apply(instance, [instance.input, o]);
      }
    };
    
    this.highlight = function(elem) {
      var item = $(elem);
      instance.unhighlight();
      if (item.length) {
        instance.selected = item;
        instance.selected.addClass('feed-ac-list-item-highlight');
      }
      if ($.isFunction(o.callbacks.highlight)) {
        o.callbacks.highlight.apply(instance, [instance.input, o]);
      }
    };
    
    this.unhighlight = function() {
      instance.selected = null;
      
      instance.selection
        .find('.feed-ac-list-item-highlight')
        .removeClass('feed-ac-list-item-highlight');
      
      if ($.isFunction(o.callbacks.unhighlight)) {
        o.callbacks.unhighlight.apply(instance, [instance.input, o]);
      }
    };

    input.bind({
      'keydown': function(e) {
        if (!e) {
          e = window.event;
        }
        switch (e.keyCode) {
          case 40: // Down arrow.
            instance.next();
            instance.viewable();
            return false;
            
          case 38: // Up arrow.
            instance.previous();
            instance.viewable();
            return false;
          
          default: // All other keys.
            return true;
        }
      },
      'keyup': function(e) {
        if (!e) {
          e = window.event;
        }
        switch (e.keyCode) {
          case 16: // Shift.
          case 17: // Ctrl.
          case 18: // Alt.
          case 20: // Caps lock.
          case 33: // Page up.
          case 34: // Page down.
          case 35: // End.
          case 36: // Home.
          case 37: // Left arrow.
          case 39: // Right arrow.
            return true;
            
          case 38: // Up arrow.
          case 40: // Down arrow.
            if (!instance.selected) {
              instance.search(instance.input.val());
            }
            return true;

          case 9:  // Tab.
          case 13: // Enter.
            instance.finalize();
            return true;
            
          case 27: // Esc.
            instance.hide();
            return true;

          default: // All other keys.
            if (instance.input.val().length > 0) {
              instance.search(instance.input.val());
            }
            else {
              instance.hide();
            }
            return true;
        }
      },
      'blur': function() {
        if (!instance.active) {
          instance.hide();
        }
      }
    })
  };

  Drupal.behaviors.FeedAutocomplete = {
    attach: function(context, settings) {

      $('.feed-category-term-autocomplete', context).each(function() {
        var autocomplete = $(this);
        var id = autocomplete.attr('id');
        var input = autocomplete.find('.feed-category-term-autocomplete-input');
        var container = autocomplete.find('.feed-category-term-autocomplete-term-container');
        var addnew = autocomplete.find('.feed-category-term-add');
        
        container.find('.feed-category-term-autocomplete-remove').live('click', function(e){
          e.preventDefault();
          
          $(this).parent().remove();
        });
        
        addnew.find('.feed-category-term-autocomplete-add').click(function() {
          var category = addnew.find('.feed-category-term-autocomplete-category option:selected');
          
          var value = category.val() + ':' + input.val();
          var title = category.text() + ': ' + input.val();

          if (!container.find('input[type="hidden"][value="' + value + '"]').length) {
            var item = $('<div />', {'class': 'feed-category-term-autocomplete-term'});
            var item_remove = $('<a href="#" class="feed-category-term-autocomplete-remove">' + settings[id]['remove_text'] + '</a>');
            var item_input = $('<input>', {'name': settings[id]['item_name'] + '[]', 'type': 'hidden'});
            var item_label = $('<span />', {'class': 'feed-category-term-autocomplete-title'});
            
            item.append(item_remove);
            item.append(item_input.val(value));
            item.append(item_label.text(title));
            item.appendTo(container);
          }
          input.val('');
          addnew.hide();
        });

        if (settings[id]) {
          var uri = settings[id]['uri'];

          new FeedAutocomplete(input, uri, {
            'callbacks': {
              'select': function(input, option, key, value) {
                if (key == 'new') {
                  addnew.show();
                }
                else {
                  item_title = value.category + ': ' + value.term;
                  item_value = key;

                  if (value && !container.find('input[type="hidden"][value="' + item_value + '"]').length) {
                    var item = $('<div />', {'class': 'feed-category-term-autocomplete-term'});
                    var item_remove = $('<a href="#" class="feed-category-term-autocomplete-remove">' + settings[id]['remove_text'] + '</a>');
                    var item_input = $('<input>', {'name': settings[id]['item_name'] + '[]', 'type': 'hidden'});
                    var item_label = $('<span />', {'class': 'feed-category-term-autocomplete-title'});

                    item.append(item_remove);
                    item.append(item_input.val(item_value));
                    item.append(item_label.text(item_title));
                    item.appendTo(container);
                  }
                  input.val('');
                }
              }
            }
          });
        }
      });
    }
  };
})(jQuery);