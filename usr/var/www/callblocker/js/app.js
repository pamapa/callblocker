/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2015 Patrick Ammann <pammann@gmx.net>

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

require(["dijit/ConfirmDialog",
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
         "dijit/form/Form",
         "dijit/form/Button",
         "dijit/form/Select",
         "dijit/form/ValidationTextBox",
         "dijit/layout/ContentPane",
         "dijit/layout/LayoutContainer",
         "dijit/layout/BorderContainer",
        ], function(ConfirmDialog) { // workaround

  function formatDate(timestamp) {
    if (timestamp) {
      var date = new Date(timestamp);
      return dojo.date.locale.format(date, {formatLength: "long"});
    }
    return "";
  }

  function createCallerLogGrid() {
    var store = new dojox.data.QueryReadStore({
      url: "callerlog.php"
    });
    var structure = [
      { name: "Date",      field: "TIMESTAMP", width:"150px", formatter: formatDate},
      { name: "Number",    field: "NUMBER",    width:"120px"},
      { name: "Name",      field: "NAME",      width:"200px"},
      { name: "Blocked",   field: "BLOCKED",   width:"120px", hidden:true},
      { name: "Whitelist", field: "WHITELIST", width:"300px"},
      { name: "Blacklist", field: "BLACKLIST", width:"300px"},
      { name: "Score",     field: "SCORE",     width:"50px"}
    ];

    var menu = new dijit.Menu();
    var addToWhitelistMenuItem = new dijit.MenuItem({
      label: "Add to whitelist",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          var listStore = createListStore("list.php?dirname=whitelists");
          dojo.forEach(items, function(si){
            if (si !== null) {
              var newItem = { timestamp: Date.now(),
                              number: grid.store.getValue(si, "NUMBER"), name: grid.store.getValue(si, "NAME")};
              listStore.newItem(newItem);
            }
          });
          listStore.save();
        } 
      }
    });
    var addToBlacklistMenuItem = new dijit.MenuItem({
      label: "Add to blacklist",
      onClick: function(){
        var items = grid.selection.getSelected();
        if (items.length) {
          var listStore = createListStore("list.php?dirname=blacklists");
          dojo.forEach(items, function(si){
            if (si !== null) {
              var newItem = { timestamp: Date.now(),
                              number: grid.store.getValue(si, "NUMBER"), name: grid.store.getValue(si, "NAME")};
              listStore.newItem(newItem);
            }
          });
          listStore.save();
        } 
      }
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
        if (item.i.BLOCKED.indexOf("blocked") != -1) {
          row.customClasses = "blockedRow";
        } else if (item.i.WHITELIST) {
          row.customClasses = "whitelistRow";
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
      { name: "Date",     field: "TIMESTAMP", width:"150px", formatter: formatDate},
      { name: "PrioId",   field: "PRIO_ID",   width:"50px", hidden:true},
      { name: "Priority", field: "PRIORITY",  width:"70px"},
      { name: "Message",  field: "MESSAGE",   width:"100%"}
    ];
    var grid = new dojox.grid.EnhancedGrid({
      //id: "grid",
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
        if (item.i.PRIO_ID <= 3) {
          row.customClasses = "errorRow";
        } else if (item.i.PRIO_ID == 4) {
          row.customClasses = "warnRow";
        }
      }
    });
    return grid;
  }

  function createJournalErrorWarnGrid() {
    return createJournalGrid("journal.php");
  }

  function createJournalAllGrid() {
    return createJournalGrid("journal.php?all=1");
  }

  function createListStore(url) {
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

  function createListX(url) {
    var numberTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "Number",
      pattern: "\\+[0-9]{4,15}",
      required: true,
      invalidMessage: "Not a valid international number (+...)"
    });
    var nameTextBox = new dijit.form.ValidationTextBox({
      placeHolder: "Name"
    });

    var listStore = createListStore(url);
    var structure = [
      { name: "Date",      field: "timestamp", width:"150px", formatter: formatDate},
      { name: "Number",    field: "number",    width:"120px"},
      { name: "Name",      field: "name",      width:"200px"}
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
            content: [numberTextBox.domNode, nameTextBox.domNode],
            onExecute:function() {
              if (numberTextBox.isValid() && nameTextBox.isValid()) {
                grid.store.setValue(si, "timestamp", Date.now());
                grid.store.setValue(si, "number", numberTextBox.get("value"));
                grid.store.setValue(si, "name", nameTextBox.get("value"));
                grid.store.save();
              }
            }
          });
          numberTextBox.set("value", grid.store.getValue(si, "number"));
          nameTextBox.set("value", grid.store.getValue(si, "name"));
          myDialog.show();
        }
      },
      //iconClass: "dijitEditorIcon dijitEditorIconDelete"
    });
    menu.addChild(deleteMenuItem);
    menu.addChild(editMenuItem);

    var grid = new dojox.grid.EnhancedGrid({
      store: listStore,
      structure: structure,
      canSort:function(){return false}, // disable sorting, its not implemented on backend
      selectable: true,
      plugins : {menus: menusObject = {rowMenu: menu}},
      style:"height:100%; width:100%;",
      region: "center",
    });
    /*dojo.connect(grid, "onKeyPress", function(evt) {
      if(evt.keyCode === dojo.keys.DELETE) { 
        console.log('delete!'); 
      }
    });*/
    var addNewEntry = new dijit.form.Button({
      label: "Add new entry",
      onClick: function() {
        var myDialog = new ConfirmDialog({
          title: "Add new entry",
          content: [numberTextBox.domNode, nameTextBox.domNode],
          onExecute:function() {
            if (numberTextBox.isValid() && nameTextBox.isValid()) {
              var newItem = {timestamp: Date.now(), number: numberTextBox.get("value"), name: nameTextBox.get("value")};
              grid.store.newItem(newItem);
              grid.store.save();
            }
          }
        });
        myDialog.show();
      },
      region: "top",
    });

    var listLayout = new dijit.layout.LayoutContainer();
    listLayout.addChild(addNewEntry);
    listLayout.addChild(grid);
    return listLayout;
  }

  function createWhitelist() {
    return createListX("list.php?dirname=whitelists");
  }

  function createBlacklist() {
    return createListX("list.php?dirname=blacklists");
  }

  function createTree() {  
    var treeData = {
      identifier: "id",
      label: "name",
      items: [
        { id: "root", name:"Root", func:null,
          children:[{_reference:"calllog"}, {_reference:"config"}, {_reference:"diag"}] 
        },
        { id: "calllog", name:"Caller Log", func:createCallerLogGrid},
        { id: "config", name:"Configuration", func:null,
          children:[{_reference:"config_whitelists"}, {_reference:"config_blacklists"}] 
        },
        { id: "config_whitelists", name:"Whitelist", func:createWhitelist},
        { id: "config_blacklists", name:"Blacklist", func:createBlacklist},
        { id: "diag", name:"Diagnostics", func:null,
          children:[{_reference:"diag_error_warn"}, {_reference:"diag_all"}] 
        },
        { id: "diag_error_warn", name:"Error/Warnings", func:createJournalErrorWarnGrid},
        { id: "diag_all", name:"All", func:createJournalAllGrid}
      ]
    };

    var treeStore = new dojo.data.ItemFileWriteStore({
      data:treeData
    });
    var treeModel = new dijit.tree.TreeStoreModel({
      id:"model",
      store:treeStore,
      childrenAttrs:["children"],
      query:{id:"root"}
    });
    var tree = new dijit.Tree({
      model:treeModel,
      persist:false,
      showRoot:false,
      onClick: function(item) {
        if (item.func[0] != null) {
          mainPane.set("content", item.func[0]());
        }
      }
    });
    return tree;
  }


  //
  // main
  //
  var appLayout = new dijit.layout.BorderContainer({
    design: "headline",
    style: "height: 100%; width: 100%;",
    gutters:true
  });

  var headerPane = new dijit.layout.ContentPane({
    region: "top",
    content: "Here comes the logo"
  });
  appLayout.addChild(headerPane);

  var menuPane = new dijit.layout.ContentPane({
    region: "left",
    style: "width: 150px",
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
    content: "Callblocker v0.0.1"
  });
  appLayout.addChild(statusbarPane);
  appLayout.placeAt(document.body);
  appLayout.startup();
});

