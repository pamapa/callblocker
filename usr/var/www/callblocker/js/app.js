/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2019 Patrick Ammann <pammann@gmx.net>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// Useful links:
// http://amagard.x10.mx/dojo_icon_classes.html
// http://kennethfranqueiro.com/2010/06/custom-save-logic-itemfilewritestore/

require(["dijit/ConfirmDialog",
         "dojo/dom-construct",
         "dojo/keys",
         "dojo/data/ItemFileWriteStore",
         "dojo/date/locale",
         "dojo/data/ItemFileWriteStore",
         "dojox/data/QueryReadStore",
         "dojox/grid/DataGrid",
         "dojox/grid/EnhancedGrid",
         "dojox/grid/enhanced/plugins/Menu",
         "dijit/Tree",
         "dijit/Menu",
         "dijit/MenuItem",
         "dijit/tree/TreeStoreModel",
         "dijit/form/CheckBox",
         "dijit/form/Button",
         "dijit/form/Select",
         "dojox/form/Uploader",
         "dijit/form/ValidationTextBox",
         "dijit/layout/ContentPane",
         "dijit/layout/LayoutContainer",
         "dijit/layout/BorderContainer",
        ], function(ConfirmDialog, domConstruct) { // workaround

  var api_base = "python-fcgi/api.py";

  function formatDate(dateStr) {
    if (dateStr) {
      // 2015-04-19 10:14:03 +0000
      // coming from backend (settings.py and journal.py)

      // convert to ISO 8601: 2015-04-19T10:14:03+0000
      dateStr = dateStr.replace(" ", "T").replace(" ", "");

      // localize date
      var date = new Date(dateStr); // RFC2822 or ISO 8601 date
      return dojo.date.locale.format(date, {formatLength: "long"});
    }
    return "";
  }

  // convert to UTC: 2015-04-19 10:14:03 +0000
  function date2UTCString(date) {
    var str =
      date.getUTCFullYear()+"-"+(date.getUTCMonth()+1)+"-"+date.getUTCDate()+" "+
      date.getUTCHours()+":"+date.getUTCMinutes()+":"+date.getUTCSeconds()+" +0000";
    return str;
  };


  // --------------------------------------------------------------------------
  // Caller Log
  // --------------------------------------------------------------------------
  function createCallerLogGrid() {
    var store = new dojox.data.QueryReadStore({
      url: api_base.concat("/callerlog")
    });
    var structure = [
      { name: "Date",   field: "date",   width: "200px", formatter: formatDate },
      { name: "Number", field: "number", width: "110px"                        },
      { name: "Name",   field: "name",   width: "600px"                        },
      { name: "What",   field: "what",   width: "50px",  hidden: true          },
      { name: "Reason", field: "reason", width: "300px"                        }
    ];

    var menu = new dijit.Menu();
    function onClickAddToList(url) {
      var items = grid.selection.getSelected();
      if (items.length) {
        var listStore = createListStore(url);
        dojo.forEach(items, function(si) {
          if (si !== null) {
            var dateStr = date2UTCString(new Date());
            var newItem = { number: grid.store.getValue(si, "number"), name: grid.store.getValue(si, "name"),
                            date_created: dateStr, date_modified: dateStr };
            listStore.newItem(newItem);
          }
        });
        listStore.save();
      } 
    }
    var addToWhitelistMenuItem = new dijit.MenuItem({
      label: "Add to whitelist",
      onClick: function() { onClickAddToList(api_base.concat("/get_list?dirname=whitelists")); }
    });
    var addToBlacklistMenuItem = new dijit.MenuItem({
      label: "Add to blacklist",
      onClick: function() { onClickAddToList(api_base.concat("/get_list?dirname=blacklists")); }
    });
    menu.addChild(addToWhitelistMenuItem);
    menu.addChild(addToBlacklistMenuItem);

    var grid = new dojox.grid.EnhancedGrid({
      store: store,
      structure: structure,
      canSort:function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      plugins : {menus: menusObject = {rowMenu: menu}},
      style:"height:100%; width:100%;"
    });
    // setup colors
    dojo.connect(grid, 'onStyleRow', this, function (row) {
      var item = grid.getItem(row.index);
      if (item != null) {
        switch(item.i.what) {
          case 1:
            row.customClasses = "whitelistRow";
            break;
          case -1:
            row.customClasses = "blockedRow";
            break;
          default:
            break;
        }
      }
    });
    return grid;
  }

  function createJournalGrid(url) {
    var store = new dojox.data.QueryReadStore({
      url: url
    });
    var structure = [
      { name: "Date",     field: "date",     width: "200px", formatter: formatDate },
      { name: "PrioId",   field: "prio_id",  width: "50px",  hidden: true          },
      { name: "Priority", field: "priority", width: "70px"                         },
      { name: "Message",  field: "message",  width: "100%"                         }
    ];
    var grid = new dojox.grid.EnhancedGrid({
      //id: "myGridId",
      store: store,
      structure: structure,
      canSort:function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      style:"height:100%; width:100%;"
    });
    // setup colors
    dojo.connect(grid, 'onStyleRow', this, function (row) {
      var item = grid.getItem(row.index);
      if (item != null) {
        if (item.i.prio_id <= 3) {
          row.customClasses = "errorRow";
        } else if (item.i.prio_id == 4) {
          row.customClasses = "warnRow";
        }
      }
    });
    return grid;
  }


  // --------------------------------------------------------------------------
  // Configuration
  // --------------------------------------------------------------------------
  function createListStore(url) {
    // console.log("createListStore: ", url);
    var store = new dojo.data.ItemFileWriteStore({
      url: url
    });
    store._saveEverything = function(saveCompleteCallback, saveFailedCallback, newFileContentString) {
      dojo.xhrPost({
        url: store.url,
        content: {data: newFileContentString},
        load: saveCompleteCallback,
        error: saveFailedCallback
      });
    }
    return store;
  }

  function createPhone() {
    var typeSelect = new dijit.form.Select({
      options: [
        { label: "SIP Phone", value: "sip", selected: true },
        { label: "Analog Phone", value: "analog" }
      ]
    });
    function selectType(isSIP) {
      //console.log("selectType: ", isSIP);
      deviceTextBox.set("disabled", isSIP);
      domainTextBox.set("disabled", !isSIP);
      usernameTextBox.set("disabled", !isSIP);
      passwordTextBox.set("disabled", !isSIP);
    }
    dojo.connect(typeSelect, "onChange", function(evt) {
      //console.log("typeSelect(onChange): ", evt);
      selectType(evt == "sip");
    });
    var enabledCheckBox = new dijit.form.CheckBox({
      checked: true,
    });
    var nameTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "name"
    });
    var countryCodeTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "country code",
      required: true,
      pattern: "\\+[0-9]{1,4}",
      invalidMessage: "Not a valid counry code (+...)"
    });
    var blockModeSelect = new dijit.form.Select({
      options: [
        { label: "logging only", value: "logging_only", selected: true },
        { label: "whitelists only", value: "whitelists_only" },
        { label: "whitelists and blacklists", value: "whitelists_and_blacklists" },
        { label: "blacklists only", value: "blacklists_only" }
      ]
    });
    function selectBlockMode(mode) {
      //console.log("selectBlockMode: ", mode);
      var isLoggingOnly = (mode == "logging_only" ? true : false);
      blockAnonymousCIDCheckBox.set("disabled", isLoggingOnly);
      blockInvalidCIDCheckBox.set("disabled", isLoggingOnly);
    }
    dojo.connect(blockModeSelect, "onChange", function(evt) {
      //console.log("blockModeSelect(onChange): ", evt);
      selectBlockMode(evt);
    });
    var blockAnonymousCIDCheckBox = new dijit.form.CheckBox({
      checked: false,
    });
    var blockInvalidCIDCheckBox = new dijit.form.CheckBox({
      checked: false,
    });
    var onlineCheckSelect = new dijit.form.Select({
      store: new dojo.data.ItemFileReadStore({
        url: api_base.concat("/get_online_scripts?kind=check")
      })
    });
    // add none and preselect
    onlineCheckSelect.addOption([{ label: "none", value: "", selected: true }]);
    var onlineLookupSelect = new dijit.form.Select({
      store: new dojo.data.ItemFileReadStore({
        url: api_base.concat("/get_online_scripts?kind=lookup")
      })
    });
    // add none and preselect
    onlineLookupSelect.addOption([{ label: "none", value: "", selected: true }]);
    var deviceTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "device",
      pattern: "/dev/.*",
      invalidMessage: "Not a valid device name (/dev/...)"
    });
    var domainTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "sip domain"
    });
    var usernameTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "sip username"
    });
    var passwordTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "sip password",
      type: "password"
    });

    var phoneStore = createListStore(api_base.concat("/phones"));
    var structure = [
      { name: "Enabled",             field: "enabled",             width: "40px",
        type:dojox.grid.cells._Widget, formatter:function(on){
          return new dijit.form.CheckBox({checked: on, readOnly: true});
        }
      },
      { name: "Name",                field: "name",                width: "100px" },
      { name: "Country code",        field: "country_code",        width: "40px"  },
      { name: "Block mode",          field: "block_mode",          width: "120px" },
      { name: "Block anonymous CID", field: "block_anonymous_cid", width: "60px",
        type:dojox.grid.cells._Widget, formatter:function(on){
          return new dijit.form.CheckBox({checked: on, readOnly: true});
        }
      },
      { name: "Block invalid CID",   field: "block_invalid_cid",   width: "60px",
        type:dojox.grid.cells._Widget, formatter:function(on){
          return new dijit.form.CheckBox({checked: on, readOnly: true});
        }
      },
      { name: "Online check",  field: "online_check",  width: "100px" },
      { name: "Online lookup", field: "online_lookup", width: "100px" },
      { name: "Device",        field: "device",        width: "100px" },
      { name: "SIP domain",    field: "domain",        width: "160px" },
      { name: "SIP username",  field: "username",      width: "160px" },
      { name: "SIP password",  field: "password",      width: "160px",
        type:dojox.grid.cells._Widget, formatter:function(str){
          return new dijit.form.TextBox({value: str, type: "password", readOnly: true});
        }
      },
    ];

    var menu = new dijit.Menu();
    var deleteMenuItem = new dijit.MenuItem({
      label: "Delete",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          dojo.forEach(items, function(si){
            if (si !== null) {
              grid.store.deleteItem(si);
            }
          });
          grid.store.save();
        }
      },
      iconClass: "dijitEditorIcon dijitEditorIconDelete"
    });
    var editMenuItem = new dijit.MenuItem({
      label: "Edit",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          var si = items[0];
          var myDialog = new ConfirmDialog({
            title: "Edit entry",
            content: [
              domConstruct.create("T", {innerHTML:"Type: "}), typeSelect.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Enabled: "}), enabledCheckBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Name: "}), nameTextBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Country code: "}), countryCodeTextBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Block mode: "}), blockModeSelect.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Block anonymous CID: "}), blockAnonymousCIDCheckBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Block invalid CID: "}), blockInvalidCIDCheckBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Online check: "}), onlineCheckSelect.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Online lookup: "}), onlineLookupSelect.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"Device: "}), deviceTextBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"SIP domain: "}), domainTextBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"SIP username: "}), usernameTextBox.domNode, domConstruct.create("br"),
              domConstruct.create("T", {innerHTML:"SIP password: "}), passwordTextBox.domNode, domConstruct.create("br"),
            ],
            onExecute:function() {
              if (!nameTextBox.isValid()) return;
              if (typeSelect.get("value") == "analog") {
                if (!deviceTextBox.isValid()) return;
                grid.store.setValue(si, "device", deviceTextBox.get("value"));
                grid.store.setValue(si, "domain", "");
                grid.store.setValue(si, "username", "");
                grid.store.setValue(si, "password", "");
              } else {
                if (!domainTextBox.isValid()) return;
                if (!usernameTextBox.isValid()) return;
                if (!passwordTextBox.isValid()) return;
                grid.store.setValue(si, "device", "");
                grid.store.setValue(si, "domain", domainTextBox.get("value"));
                grid.store.setValue(si, "username", usernameTextBox.get("value"));
                grid.store.setValue(si, "password", passwordTextBox.get("value"));
              }
              grid.store.setValue(si, "enabled", enabledCheckBox.get("checked"));
              grid.store.setValue(si, "name", nameTextBox.get("value"));
              grid.store.setValue(si, "country_code", countryCodeTextBox.get("value"));

              grid.store.setValue(si, "block_mode", blockModeSelect.get("value"));
              grid.store.setValue(si, "block_anonymous_cid", blockAnonymousCIDCheckBox.get("checked"));
              grid.store.setValue(si, "block_invalid_cid", blockInvalidCIDCheckBox.get("checked"));

              grid.store.setValue(si, "online_check", onlineCheckSelect.get("value"));
              grid.store.setValue(si, "online_lookup", onlineLookupSelect.get("value"));

              grid.store.save();
            }
          });

          // prepare dialog fields
          enabledCheckBox.set("value", grid.store.getValue(si, "enabled"));
          nameTextBox.set("value", grid.store.getValue(si, "name"));
          countryCodeTextBox.set("value", grid.store.getValue(si, "country_code", ""));

          var blockMode = grid.store.getValue(si, "block_mode", "logging_only")
          blockModeSelect.set("value", blockMode);
          blockAnonymousCIDCheckBox.set("value", grid.store.getValue(si, "block_anonymous_cid", false));
          blockInvalidCIDCheckBox.set("value", grid.store.getValue(si, "block_invalid_cid", true));
          selectBlockMode(blockMode);

          onlineCheckSelect.set("value", grid.store.getValue(si, "online_check", ""));
          onlineLookupSelect.set("value", grid.store.getValue(si, "online_lookup", ""));

          var isSIP = !grid.store.getValue(si, "device");
          if (isSIP) {
            typeSelect.set("value", "sip");
            domainTextBox.set("value", grid.store.getValue(si, "domain", ""));
            usernameTextBox.set("value", grid.store.getValue(si, "username", ""));
            passwordTextBox.set("value", grid.store.getValue(si, "password", ""));
          }
          else {
            typeSelect.set("value", "analog");
            deviceTextBox.set("value", grid.store.getValue(si, "device", ""));
          }
          selectType(isSIP);

          myDialog.show();
        }
      },
      iconClass: "iconClass dijitIconEdit"
    });
    menu.addChild(deleteMenuItem);
    menu.addChild(editMenuItem);

    var grid = new dojox.grid.EnhancedGrid({
      store: phoneStore,
      structure: structure,
      canSort:function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      plugins : {menus: menusObject = {rowMenu: menu}},
      style:"height:100%; width:100%;",
    });

    var addNewEntryButton = new dijit.form.Button({
      label: "Add new entry",
      onClick: function() {
        var newItem = {enabled: false, name: "New Phone"};
        grid.store.newItem(newItem);
        grid.store.save();
      }
    });

/*
    grid.layout.setColumnVisibility(7, false);
    grid.layout.setColumnVisibility(8, false);
    grid.layout.setColumnVisibility(9, false);
*/
    return [addNewEntryButton.domNode, domConstruct.create("br"), grid.domNode]
  }

  function createOnlineCredentials(url) {
    var nameSelect = new dijit.form.Select({
      store: new dojo.data.ItemFileReadStore({
        url: api_base.concat("/get_online_scripts?kind=check")
      })
    });
    var usernameTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "username",
      required: true,
    });
    var passwordTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "password",
      type: "password"
    });

    var menu = new dijit.Menu();
    var deleteMenuItem = new dijit.MenuItem({
      label: "Delete",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          dojo.forEach(items, function(si){
            if (si !== null) {
              grid.store.deleteItem(si);
            }
          });
          grid.store.save();
        } 
      },
      iconClass: "dijitEditorIcon dijitEditorIconDelete"
    });
    var editMenuItem = new dijit.MenuItem({
      label: "Edit",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          var si = items[0];
          var myDialog = new ConfirmDialog({
            title: "Edit entry",
            content: [nameSelect.domNode, usernameTextBox.domNode, passwordTextBox.domNode],
            onExecute: function() {
              if (usernameTextBox.isValid() && passwordTextBox.isValid()) {
                grid.store.setValue(si, "name", nameSelect.get("value"));
                grid.store.setValue(si, "username", usernameTextBox.get("value"));
                grid.store.setValue(si, "password", passwordTextBox.get("value"));
                grid.store.save();
              }
            }
          });
          // prepare dialog fields
          nameSelect.set("value", grid.store.getValue(si, "name"));
          usernameTextBox.set("value", grid.store.getValue(si, "username"));
          passwordTextBox.set("value", grid.store.getValue(si, "password"));
          myDialog.show();
        }
      },
      iconClass: "iconClass dijitIconEdit"
    });
    menu.addChild(deleteMenuItem);
    menu.addChild(editMenuItem);

    var listStore = createListStore(api_base.concat("/online_credentials"));
    var structure = [
      { name: "Name",     field: "name",     width: "150px" },
      { name: "Username", field: "username", width: "120px" },
      { name: "Password", field: "password", width: "200px",
        type:dojox.grid.cells._Widget, formatter:function(str){
          return new dijit.form.TextBox({value: str, type: "password", readOnly: true});
        }
      }
    ];
    var grid = new dojox.grid.EnhancedGrid({
      store: listStore,
      structure: structure,
      canSort: function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      plugins : {menus: menusObject = {rowMenu: menu}},
      style: "height:100%; width:100%;"
    });

    var addNewEntryButton = new dijit.form.Button({
      label: "Add new entry",
      onClick: function() {
        var myDialog = new ConfirmDialog({
          title: "Add new entry",
          content: [nameSelect.domNode, usernameTextBox.domNode, passwordTextBox.domNode],
          onExecute:function() {
            if (usernameTextBox.isValid() && passwordTextBox.isValid()) {
              var newItem = {name: nameSelect.get("value"), username: usernameTextBox.get("value"), password: passwordTextBox.get("value")};
              grid.store.newItem(newItem);
              grid.store.save();
            }
          }
        });
        myDialog.show();
      }
    });

    return [addNewEntryButton.domNode, domConstruct.create("br"), grid.domNode]
  }

  function createListX(url_param) {
    var numberTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "Number",
      required: true,
      validator: function(value, constraints) {
        var re = new RegExp("^\\+[0-9]{4,15}$");
        if (!re.test(value)) {
          this.invalidMessage = "Not a valid international number (E.164; e.g. +...)";
          return false;
        }

        var items = grid.selection.getSelected();
        if (items.length == 0 || value != grid.store.getValue(items[0], "number")) {
          // not current selected entry
          var alreadyInStore = false;
          function onComplete(items, request) {
            if(items.length !== 0) {
              alreadyInStore = true;
            }
          }
          grid.store.fetch({query: {"number": value}, onComplete: onComplete});
          if (alreadyInStore) {
            this.invalidMessage = "Number exists already";
            return false;
          }
        }

        return true;
      }
    });

    var nameTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "Name"
    });

    var menu = new dijit.Menu();
    var deleteMenuItem = new dijit.MenuItem({
      label: "Delete",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          dojo.forEach(items, function(si){
            if (si !== null) {
              grid.store.deleteItem(si);
            }
          });
          grid.store.save();
        } 
      },
      iconClass: "dijitEditorIcon dijitEditorIconDelete"
    });
    var editMenuItem = new dijit.MenuItem({
      label: "Edit",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          var si = items[0];
          var myDialog = new ConfirmDialog({
            title: "Edit entry",
            content: [numberTextBox.domNode, nameTextBox.domNode],
            onExecute: function() {
              if (numberTextBox.isValid() && nameTextBox.isValid()) {
                if (numberTextBox.get("value") == grid.store.getValue(si, "number"))
                {
                  grid.store.setValue(si, "date_modified", date2UTCString(new Date()));
                  grid.store.setValue(si, "name", nameTextBox.get("value"));
                }
                else
                {
                  // number changed, needs special handling, because number is used as identifier
                  var newItem = { number: numberTextBox.get("value"), name: nameTextBox.get("value"),
                                  date_created: grid.store.getValue(si, "date_created"), date_modified: date2UTCString(new Date()) };
                  grid.store.newItem(newItem);
                  grid.store.deleteItem(si);
                }
                grid.store.save();
              }
            }
          });
          // prepare dialog fields
          numberTextBox.set("value", grid.store.getValue(si, "number"));
          nameTextBox.set("value", grid.store.getValue(si, "name"));
          myDialog.show();
        }
      },
      iconClass: "iconClass dijitIconEdit"
    });
    menu.addChild(deleteMenuItem);
    menu.addChild(editMenuItem);

    var structure = [
      { name: "Date (modified)", field: "date_modified", width: "200px", formatter: formatDate },
      { name: "Number",          field: "number",        width: "110px"                        },
      { name: "Name",            field: "name",          width: "600px"                        }
    ];
    var grid = new dojox.grid.EnhancedGrid({
      //store: added later (see dojo.connect(listSelect...))
      structure: structure,
      // sorting done in forntend:
      //canSort: function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      plugins: {menus: menusObject = {rowMenu: menu}},
      style: "height:97%; width:100%;"
    });
    /*dojo.connect(grid, "onKeyPress", function(evt) {
      if(evt.keyCode === dojo.keys.DELETE) { 
        console.log('delete!'); 
      }
    });*/

    var listsStore = createListStore(api_base.concat("/get_lists?", url_param));
    var listSelect = new dijit.form.Select({
      store: listsStore,
      placeHolder: "Select a list",
      style: "width:150px",
    });
    dojo.connect(listSelect, "onChange", function(evt) {
      //console.log("dojo.connect");
      // force load
      var listStore = createListStore(api_base.concat("/get_list?", url_param, "&filename=", evt));
      grid.setStore(listStore);
      // allow/disallow features
      if (evt == "main.json") {
        // allow editing
        deleteMenuItem.setDisabled(false);
        editMenuItem.setDisabled(false);
        addNewEntryButton.setDisabled(false);
        importAddressbookUploader.setDisabled(false);
      } else if (evt == "import.json") {
        // only allow delete and import
        deleteMenuItem.setDisabled(false);
        editMenuItem.setDisabled(true);
        addNewEntryButton.setDisabled(true);
        importAddressbookUploader.setDisabled(false);
      } else {
        // disallow editing
        deleteMenuItem.setDisabled(true);
        editMenuItem.setDisabled(true);
        addNewEntryButton.setDisabled(true);
        importAddressbookUploader.setDisabled(true);
      }
    });
    // pre select main
    listSelect.set("value", "main.json");

    var addNewEntryButton = new dijit.form.Button({
      label: "Add new entry",
      onClick: function() {
        var myDialog = new ConfirmDialog({
          title: "Add new entry",
          content: [numberTextBox.domNode, nameTextBox.domNode],
          onExecute: function() {
            if (numberTextBox.isValid() && nameTextBox.isValid()) {
              var dateStr = date2UTCString(new Date());
              var newItem = { number: numberTextBox.get("value"), name: nameTextBox.get("value"),
                              date_created: dateStr, date_modified: dateStr };
              grid.store.newItem(newItem);
              grid.store.save();
            }
          }
        });
        // prepare dialog fields
        numberTextBox.set("value", "");
        nameTextBox.set("value", "");
        myDialog.show();
      }
    });

    var importAddressbookUploader = new dojox.form.Uploader({
      label:"Import Addressbook (merge)",
      //url: added later (see dojo.connect(listSelect...))
      multiple: false,
      uploadOnSelect: true
    });
    dojo.connect(listSelect, "onChange", function(evt) {
      //console.log("dojo.connect");
      importAddressbookUploader.set("url", api_base.concat("/get_lists?", url_param, "&import=addressbook", "&merge=", "import.json"));
    });
    dojo.connect(importAddressbookUploader, "onComplete", function(evt) {
      //console.log("dojo.connect");
      // force reload
      listSelect.setStore(createListStore(api_base.concat("/get_lists?", url_param)));
      listSelect.set("value", "import.json");
      grid.setStore(createListStore(api_base.concat("/get_list?", url_param, "&filename=", listSelect.get("value"))));
    });

    return [
      listSelect.domNode,
      addNewEntryButton.domNode,
      importAddressbookUploader.domNode,
      domConstruct.create("br"),
      grid.domNode
    ]
  }

  function createWhitelist() {
    return createListX("dirname=whitelists");
  }

  function createBlacklist() {
    return createListX("dirname=blacklists");
  }


  // --------------------------------------------------------------------------
  // Diagnostic
  // --------------------------------------------------------------------------
  function createJournalErrorWarnGrid() {
    return createJournalGrid(api_base.concat("/journal"));
  }

  function createJournalAllGrid() {
    return createJournalGrid(api_base.concat("/journal?all=1"));
  }

  function createCacheListX(url_param) {
    var menu = new dijit.Menu();
    var deleteMenuItem = new dijit.MenuItem({
      label: "Delete",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          dojo.forEach(items, function(si){
            if (si !== null) {
              grid.store.deleteItem(si);
            }
          });
          grid.store.save();
        }
      },
      iconClass: "dijitEditorIcon dijitEditorIconDelete"
    });
    menu.addChild(deleteMenuItem);

    var structure = [
      { name: "Date (created)", field: "date_created", width: "200px", formatter: formatDate },
      { name: "Number",         field: "number",       width: "110px"                        },
      { name: "Name",           field: "name",         width: "600px"                        }
    ];
    var cacheStore = createListStore(api_base.concat("/get_list?dirname=cache&", url_param));
    var grid = new dojox.grid.EnhancedGrid({
      store: cacheStore,
      structure: structure,
      //canSort: function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      plugins: {menus: menusObject = {rowMenu: menu}},
      style: "height:97%; width:100%;"
    });

    return [
      grid.domNode
    ]
  }

  function createOnlineLookuplist() {
    return createCacheListX("filename=onlinelookup.json");
  }

  function createOnlineChecklist() {
   return createCacheListX("filename=onlinecheck.json");
  }


  // --------------------------------------------------------------------------
  // Main
  // --------------------------------------------------------------------------
  function createTree() {  
    var treeData = {
      identifier: "id",
      label: "name",
      items: [
        { id: "root", name: "Root", func: null,
          children: [{_reference: "calllog"}, {_reference: "config"}, {_reference: "diag"}]
        },
        { id: "calllog", name: "Caller Log", func: createCallerLogGrid},

        // Configuration
        { id: "config", name: "Configuration", func: null,
          children: [
            {_reference: "config_phone"}, {_reference: "config_onlinecreds"},
            {_reference: "config_whitelists"}, {_reference: "config_blacklists"}
          ] 
        },
        { id: "config_phone", name:"Phone", func:createPhone},
        { id: "config_onlinecreds", name:"Online Credentials", func: createOnlineCredentials},
        { id: "config_whitelists", name:"Whitelists", func: createWhitelist},
        { id: "config_blacklists", name:"Blacklists", func: createBlacklist},

        // Diagnostic
        { id: "diag", name: "Diagnostic", func: null,
          children: [{_reference: "diag_cache"}, {_reference: "diag_logs"}]
        },
        { id: "diag_cache", name: "Cache", func: null,
          children:[
            {_reference: "diag_cache_onlinelookup"}, {_reference: "diag_cache_onlinecheck"}
          ]
        },
        { id: "diag_cache_onlinelookup", name: "Online Lookup", func: createOnlineLookuplist},
        { id: "diag_cache_onlinecheck", name: "Online Check", func: createOnlineChecklist},
        { id: "diag_logs", name: "Logs", func: null,
          children:[
            {_reference: "diag_logs_error_warn"}, {_reference: "diag_logs_all"}
          ]
        },
        { id: "diag_logs_error_warn", name: "Error and Warning", func: createJournalErrorWarnGrid},
        { id: "diag_logs_all", name: "All", func: createJournalAllGrid}
      ]
    };

    var treeStore = new dojo.data.ItemFileWriteStore({
      data:treeData
    });
    var treeModel = new dijit.tree.TreeStoreModel({
      id: "model",
      store: treeStore,
      childrenAttrs: ["children"],
      query: {id: "root"}
    });
    var tree = new dijit.Tree({
      model: treeModel,
      persist: false,
      showRoot: false,
      onClick: function(item) {
        if (item.func[0] != null) {
          mainPane.set("content", item.func[0]());
        }
      }
    });
    return tree;
  }

  var appLayout = new dijit.layout.BorderContainer({
    design: "headline",
    style: "height: 100%; width: 100%;",
    gutters:true
  });

  var headerPane = new dijit.layout.ContentPane({
    region: "top",
    style: "text-align: center",
    content: "<b>callblocker</b>"
  });
  appLayout.addChild(headerPane);

  var menuPane = new dijit.layout.ContentPane({
    region: "left",
    style: "width: 190px",
    splitter:true,
    content: createTree()
  });
  appLayout.addChild(menuPane);
  var mainPane = new dijit.layout.ContentPane({
    region: "center",
    content: "Welcome to the callblocker UI"
  });
  appLayout.addChild(mainPane);

  var statusbarPane = new dijit.layout.ContentPane({
    region: "bottom",
    style: "text-align: center",
    content: "v0.12.2"
  });
  appLayout.addChild(statusbarPane);
  appLayout.placeAt(document.body);
  appLayout.startup();
});
