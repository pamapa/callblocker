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

var path = require("path");
var webpack = require("webpack");

var DojoWebpackPlugin = require("dojo-webpack-plugin");
var CopyWebpackPlugin = require("copy-webpack-plugin");
var UglifyJsPlugin = require("uglifyjs-webpack-plugin");

module.exports = (env, argv) => {
  const isDevMode = !!(env||{}).dev;
	return {
		context: __dirname,
		entry: "src/bootstrap",
		output: {
			path: path.join(__dirname, "dist"),
			publicPath: "dist/",
			pathinfo: true,
			filename: "bundle.js"
		},
		module: {
			rules: [{
		    test: /\.(png)|(gif)$/,
		    use: [
		      {
		        loader: 'url-loader',
		        options: {
		          limit: 100000
		        }
		      }
		    ]
		  }]
		},
		plugins: [
      // public/index.html
      new CopyWebpackPlugin([{
				from: "public",
				to: "."
			}]),

      // JavaScript
			new DojoWebpackPlugin({
				loaderConfig: require("./src/loaderConfig"),
				environment: {dojoRoot: "dist"},	// used at run time for non-packed resources (e.g. blank.gif)
				buildEnvironment: {dojoRoot: "node_modules"}, // used at build time
				locales: ["en"],
				noConsole: true
			}),

			// Copy non-packed resources needed by the app to the release directory
			new CopyWebpackPlugin([{
				context: "node_modules",
				from: "dojo/resources/blank.gif",
				to: "dojo/resources"
			}]),

			// For plugins registered after the dojo-webpack-plugin, data.request has been normalized and
			// resolved to an absMid and loader-config maps and aliases have been applied
			new webpack.NormalModuleReplacementPlugin(/^dojox\/gfx\/renderer!/, "dojox/gfx/canvas"),
			new webpack.NormalModuleReplacementPlugin(
				/^css!/, function(data) {
					data.request = data.request.replace(/^css!/, "!style-loader!css-loader!less-loader!")
				}
			)
		],
		resolveLoader: {
			modules: ["node_modules"]
		},
		mode: isDevMode ? 'development' : 'production',
		optimization: {
			namedModules: false,
			splitChunks: false,
			minimizer: isDevMode ? [] : [
	      new UglifyJsPlugin({
	        cache: true,
	        parallel: true,
	        uglifyOptions: {
	          compress: true,
	          mangle: true,
						output: {comments: false}
	        },
	        sourceMap: false
	      })
	    ],
	  },
    performance: { hints: false },
		devtool: "#source-map"
	};
};
