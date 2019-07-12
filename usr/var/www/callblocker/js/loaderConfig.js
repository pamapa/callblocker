/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2019-2019 Patrick Ammann <pammann@gmx.net>

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

function getConfig(env) {
	// env is set by the 'buildEnvronment' and/or 'environment' plugin options (see webpack.config.js),
	// or by the code at the end of this file if using without webpack
	dojoConfig = {
		baseUrl: '.',
		packages: [
			{
				name: 'dojo',
				location: env.dojoRoot + '/dojo',
				lib: '.'
			},
			{
				name: 'dijit',
				location: env.dojoRoot + '/dijit',
				lib: '.'
			},
			{
				name: 'dojox',
				location: env.dojoRoot + '/dojox',
				lib: '.'
			}
		],

		paths: {
			js: "js"/*,
			theme: "theme",
			// With the webpack build, the css loader plugin is replaced by a webpack loader
			// via webpack.config.js, so the following are used only by the unpacked app.
			css: "//chuckdumont.github.io/dojo-css-plugin/1.0.0/css",
			// lesspp is used by the css loader plugin when loading LESS modules
			lesspp: "//cdnjs.cloudflare.com/ajax/libs/less.js/1.7.3/less.min",*/
		},

		deps: ["js/bootstrap"],

		async: true,

		has: {'dojo-config-api': 0},	// Don't need the config API code in the embedded Dojo loader

		fixupUrl: function(url) {
			// Load the uncompressed versions of dojo/dijit/dojox javascript files when using the dojo loader.
			// When using a webpack build, the dojo loader is not used for loading javascript files so this
			// property has no effect.  This is only needed because we're loading Dojo from a CDN for this
			// demo.  In a normal development envorinment, Dojo would be installed locally and this wouldn't
			// be needed.
			if (/\/(dojo|dijit|dojox)\/.*\.js$/.test(url)) {
			  url += ".uncompressed.js";
		  }
			return url;
		}
	};
	return dojoConfig;
}
// For Webpack, export the config.  This is needed both at build time and on the client at runtime
// for the packed application.
if (typeof module !== 'undefined' && module) {
	module.exports = getConfig;
} else {
	// No webpack.  This script was loaded by page via script tag, so load Dojo from CDN
	getConfig({dojoRoot: '//ajax.googleapis.com/ajax/libs/dojo/1.14.1'});
}
