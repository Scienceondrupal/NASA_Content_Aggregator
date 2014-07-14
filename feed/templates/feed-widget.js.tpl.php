(function(window, document, undefined) {

var FeedWidgetTemplates = {
  'default': '.feed-widget { font-size: 12px; font-family: arial, sans-serif; } .feed-widget-info-field-name, .feed-widget-item-field-name { display: none; } .feed-widget-item { border-top: 1px solid #99B1D9; padding: 5px 0; } .feed-widget-item-container .first { border-top: none; } .feed-widget-item-field-value { font-size: 1em } .feed-widget-pages-next, .feed-widget-pages-previous { color: #00e; cursor: pointer; margin-right: 10px; font-weight: bold; } #feed-widget-info-field-title .feed-widget-info-field-value, #feed-widget-info-field-title a { color: #4975BB; text-decoration: none; font-weight: bold; font-size: 1.3em; } #feed-widget-info-field-title a { color: #4975BB; text-decoration: none; } #feed-widget-item-field-title a { color: #00e; text-decoration: none; } #feed-widget-item-field-description { margin: 5px 0; }'
};

var FeedWidgetToolkit = {
  'script': function(src, callback) {
    var script = document.createElement('script');
    var head = document.getElementsByTagName('head');
    var complete = false;
      
    script.type = 'text/javascript';
    script.src = src;
    script.defer = 'defer';
    script.onload = script.onreadystatechange = function() {
      if (!complete && (!window.readyState || this.readyState == 'loaded' || this.readyState == 'complete')) {
        complete = true;
        script.onload = script.onreadystatechange = null;
        if (typeof(callback) == 'function') callback();
      }
    };
    head[0].appendChild(script);
  },
  'check': function(required, check) {
    var required = required.split('.');
    var check = check.split('.');
    
    for (i in required) {
      if (typeof(check[i]) == 'undefined') {
        return false;
      }
      var r = parseInt(required[i]);
      var c = parseInt(check[i]);
      
      if (r > c) {
        return false;
      } else if (c > r) {
        return true;
      }
      if ((required.length-1) == i && required.length > check.length) {
        return true;
      }
    }
    return true;
  },
  'init': function($, elem, options) {
    var initializing = false, fnTest = /xyz/.test(function(){xyz;}) ? /\b_super\b/ : /.*/;
    
    var FeedWidgetBase = function(){};
    
    FeedWidgetBase.extend = function(prop) {
      var _super = this.prototype;
      
      initializing = true;
      var prototype = new this();
      initializing = false;
      
      for (var name in prop) {
        prototype[name] = typeof prop[name] == "function" &&
          typeof _super[name] == "function" && fnTest.test(prop[name]) ?
          (function(name, fn) {
            return function() {
              var tmp = this._super;
              this._super = _super[name];
              var ret = fn.apply(this, arguments);       
              this._super = tmp;
             
              return ret;
            };
          })(name, prop[name]) :
          prop[name];
      }
      
      function FeedWidgetBase() {
        if ( !initializing && this.init )
          this.init.apply(this, arguments);
      }
     
      FeedWidgetBase.prototype = prototype;
      FeedWidgetBase.prototype.constructor = FeedWidgetBase;
      FeedWidgetBase.extend = arguments.callee;
     
      return FeedWidgetBase;
    };

    FeedWidget = FeedWidgetBase.extend({
      'init': function(container, o) {
        this.container = $(container).addClass('feed-widget');
        this.options = $.extend(true, {
          'source': '',
          'format': null,
          'show': {
            'infos': ['title', 'description'],
            'items': ['title', 'description'],
          },
          'template': 'default',
          'page': '1',
          'limit': '5',
          'paginate': true,
          'sort': 'published',
          'order': 'descending',
          'random': false,
        }, o || {});
        
        if (!this.options.format) {
          this.options.format = {
            'infos': {
              'title': 'text',
              'link': 'text',
              'description': 'text'
            },
            'items': {
              'title': 'text',
              'link': 'text',
              'description': 'text'
            }
          };
        }
        console.log(this.options);
        if ($.isFunction(this.configure)) {
          this.configure();
        }
        this.elements = this.elements || {};
        
        this.initialize();
      },
      'templates': {
        'info-container': '<div class="feed-widget-info-container"></div>',
        'info-field': '<div class="feed-widget-info-field"></div>',
        'info-field-name': '<div class="feed-widget-info-field-name"></div>',
        'info-field-value-container': '<div class="feed-widget-info-field-value-container"></div>',
        'info-field-value': '<div class="feed-widget-info-field-value"></div>',
        'item-container': '<div class="feed-widget-item-container"></div>',
        'item': '<div class="feed-widget-item"></div>',
        'item-field': '<div class="feed-widget-item-field"></div>',
        'item-field-name': '<div class="feed-widget-item-field-name"></div>',
        'item-field-value-container': '<div class="feed-widget-item-field-value-container"></div>',
        'item-field-value': '<div class="feed-widget-item-field-value"></div>',
        'page-container': '<div class="feed-widget-pagination-container"></div>',
        'page-previous': '<a class="feed-widget-pagination-previous"></a>',
        'page-next': '<a class="feed-widget-pagination-next"></a>',
        'page-pages': '<span class="feed-widget-pagination-list"></span>'
      },
      'option': function(option, value) {
        if (typeof(value) !== 'undefined') {
          this.options[option] = value;
        }
        return this.options[option] || null;
      },
      'filter': {
        'infos': function(data) {
          if (data['link'] && data['title']) {
            for (var index in data['link']) {
              if (data['title'][index]) {
                data['title'][index] = $('<a />', {
                  'href': data['link'][index],
                  'text': data['title'][index]
                });
                delete data['link'][index];
              }
            }
            if ($.isEmptyObject(data['link'])) {
              delete data['link'];
            }
          }
          return data;
        },
        'items': function(datas) {
          for (var item in datas) {
            if (!datas[item]['link'] || !datas[item]['title']) continue;
            
            for (var index in datas[item]['link']) {
              if (!datas[item]['title'][index]) continue;
              
              datas[item]['title'][index] = $('<a />', {
                'href': datas[item]['link'][index],
                'text': datas[item]['title'][index]
              });
              delete datas[item]['link'][index];
            }
            if ($.isEmptyObject(datas[item]['link'])) {
              delete datas[item]['link'];
            }
          }
          return datas;
        }
      },
      'configure': function() {
        this.elements = {};
        this.elements['infos'] = $(this.templates['info-container']).appendTo(this.container);
        this.elements['items'] = $(this.templates['item-container']).appendTo(this.container);
        
        if (this.options.paginate) {
          this.elements['pages'] = $(this.templates['page-container']).appendTo(this.container);
        }
        
        this.build = $.extend(true, {
          'infos': function(data) {
            this.elements['infos'].children().remove();

            for (var name in data) {
              if ($.inArray(name, this.options.show.infos) == -1) {
                continue;
              }
              var values = data[name];
              
              var info_id = 'feed-widget-info-field-' + name.replace('/[^a-z0-9-_]+/i', '-');
              var info_field = $(this.templates['info-field']).attr('id', info_id);
              var info_field_name = $(this.templates['info-field-name']).text(name);
              var info_field_values = $(this.templates['info-field-value-container']);
              
              if (values.length) {
                info_field.append(info_field_name);
                info_field.append(info_field_values);
                
                for (var index in values) {
                  info_field_values.append($(this.templates['info-field-value']).html(values[index]));
                };
              }
              this.elements['infos'].append(info_field);
            };
          },
          'items': function(data) {
            this.elements['items'].children().remove();

            if ($.isArray(data)) {
              var count = 1;

              for (var index in data) {
                var item = this.build['item-element'].call(this, data[index]);
                
                if (count == 1) {
                  item.addClass('first');
                }
                if (count == data.length) {
                  item.addClass('last');
                }
                item.addClass(count % 2 ? 'even' : 'odd');
                
                this.elements['items'].append(item); count++;
              };
            }
          },
          'item-element': function(data) {
            var element = $(this.templates['item']);

            for (var name in data) {
              if ($.inArray(name, this.options.show.items) == -1) {
                continue;
              }
              var values = data[name];
              
              var item_id = 'feed-widget-item-field-' + name.replace('/[^a-z0-9-_]+/i', '-');
              var item_field = $(this.templates['item-field']).attr('id', item_id);
              var item_field_name = $(this.templates['item-field-name']).text(name);
              var item_field_values = $(this.templates['item-field-value-container']);

              if (values.length) {
                item_field.append(item_field_name);
                item_field.append(item_field_values);
                
                for (var index in values) {
                  var item_field_value = $(this.templates['item-field-value']).html(values[index]);
                  
                  item_field_values.append(item_field_value);
                };
              }
              element.append(item_field);
            };
            
            return element;
          },
          'pages': function(data) {
            if (!this.elements['pages']) return false;

            var instance = this;
            var previous = parseInt(data.previous);
            var next = parseInt(data.next);
            var pages = parseInt(data.pages);
            
            this.elements['pages'].children().remove();

            if (!isNaN(previous) && previous > 0) {
              this.elements['pages'].append($('<span />', {
                'class': 'feed-widget-pages-previous',
                'text': 'previous',
                'click': function(e) {
                  instance.loadpage(previous);
                }
              }));
            }
            if (!isNaN(next) && next <= pages) {
              this.elements['pages'].append($('<span />', {
                'class': 'feed-widget-pages-next',
                'text': 'next',
                'click': function(e) {
                  instance.loadpage(next);
                }
              }));
            }
          }
        }, this.build || {});
      },
      'refresh': function(datas) {
        for (var name in datas) {
          var data = datas[name];
          
          if (this.filter[name] && $.isFunction(this.filter[name])) {
            data = this.filter[name].call(this, data);
          }
          if (this.elements[name] && this.build[name] && $.isFunction(this.build[name])) {
            this.build[name].call(this, data);
          }
        };
      },
      'initialize': function() {
        if (this.options.template && typeof(FeedWidgetTemplates[this.options.template]) == 'string') {
          $('head').append('<style type="text/css">' + FeedWidgetTemplates[this.options.template] + '</style>');
        }
        this.loadpage(this.options.page);
      },
      'loadpage': function(page) {
        this.load(this.refresh, {
          'page': page,
          'format': this.options.format,
          'show': this.options.show,
          'limit': this.options.limit,
          'sort': this.options.sort,
          'order': this.options.order,
          'random': this.options.random,
        });
      },
      'load': function(parser, params) {
        if ($.isFunction(parser)) {
          $.ajax({
            'url': this.options.source,
            'dataType': 'jsonp',
            'context': this,
            'data': params,
            'success': function(data) {
              parser.call(this, data);
            }
          });
        }
      }
    });
    
    $(function(){
      new FeedWidget(elem, options);
    });
  }
};

window.FeedWidgetInitialize = function(elem, options) {
  if (typeof(jQuery) == 'undefined' || FeedWidgetToolkit.check('1.6.2', jQuery.fn.jquery)) {
    FeedWidgetToolkit.script('https://ajax.googleapis.com/ajax/libs/jquery/1.6.2/jquery.min.js', function() {
      if (typeof(jQuery) != 'undefined' && FeedWidgetToolkit.check('1.6.2', jQuery.fn.jquery)) {
        jQuery_1_6_2 = jQuery.noConflict(true);
        FeedWidgetToolkit.init(jQuery_1_6_2, elem, options);
      }
    });
  } else {
    FeedWidgetToolkit.init(jQuery, elem, options);
  }
};

})(window, document, undefined);

new FeedWidgetInitialize('<?php print $selector ?>', <?php print drupal_json_encode($configuration) ?>);