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

require(["dojo/data/ItemFileWriteStore",
         "dijit/tree/TreeStoreModel",
         "dijit/Tree",
         "dojox/data/QueryReadStore",
         "dojox/grid/DataGrid",
         "dijit/layout/ContentPane",
         "dijit/layout/BorderContainer",
        ], function() {

  function createCallerLogGrid() {
    var store = new dojox.data.QueryReadStore({
      url: "callerlog.php"
    });
    var structure = [
      { name: "Date",      field: "DATE",      width:"120px"},
      { name: "Number",    field: "NUMBER",    width:"120px"},
      { name: "Name",      field: "NAME",      width:"200px"},
      { name: "Blocked",   field: "BLOCKED",   width:"120px", hidden:true},
      { name: "Whitelist", field: "WHITELIST", width:"300px"},
      { name: "Blacklist", field: "BLACKLIST", width:"300px"},
      { name: "Score",     field: "SCORE",     width:"50px"}
    ];
    var grid = new dojox.grid.DataGrid({
      store: store,
      structure: structure,
      style:"height:100%; width:100%;",
      canSort:function(){return false} // disable sorting, its not implemented on backend
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
      { name: "Date",     field: "DATE",     width:"120px"},
      { name: "PrioId",   field: "PRIO_ID",  width:"120px", hidden:true},
      { name: "Priority", field: "PRIORITY", width:"120px"},
      { name: "Message",  field: "MESSAGE",  width:"100%"}
    ];
    var grid = new dojox.grid.DataGrid({
      //id: "grid",
      store: store,
      structure: structure,
      style:"height:100%; width:100%;",
      canSort:function(){return false} // disable sorting, its not implemented on backend
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

  function createMenu() {  
    var menuData = {
      identifier: "id",
      label: "name",
      items: [
        { id: "root", name:"Root", func:null,
          children:[/*{_reference:"todo"},*/ {_reference:"calllog"}, {_reference:"diag"}] 
        },
        //{ id: "todo", name:"Setup", func:null },
        { id: "calllog", name:"Caller Log", func:createCallerLogGrid},
        { id: "diag", name:"Diagnostics", func:null,
          children:[{_reference:"diag_error_warn"}, {_reference:"diag_all"}] 
        },
        { id: "diag_error_warn", name:"Error/Warnings", func:createJournalErrorWarnGrid},
        { id: "diag_all", name:"All", func:createJournalAllGrid}
      ]
    };

    var menuStore = new dojo.data.ItemFileWriteStore({
      data:menuData
    });
    var menuModel = new dijit.tree.TreeStoreModel({
      id:"model",
      store:menuStore,
      childrenAttrs:["children"],
      query:{id:"root"}
    });
    var menu = new dijit.Tree({
      model:menuModel,
      persist:false,
      showRoot:false,
      onClick: function(item) {
        if (item.func[0] != null) {
          mainPane.set("content", item.func[0]());
        }
      }
    });
    return menu;
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
    content: createMenu()
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

