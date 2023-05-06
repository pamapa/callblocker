/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2019-2020 Patrick Ammann <pammann@gmx.net>

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

const path = require("path");
const webpack = require("webpack");

// plugins
const { CleanWebpackPlugin } = require("clean-webpack-plugin");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const DojoWebpackPlugin = require("dojo-webpack-plugin");
const CopyWebpackPlugin = require("copy-webpack-plugin");
const TerserPlugin = require("terser-webpack-plugin");

module.exports = env => {
  const devmode = !!(env||{}).dev;
  console.log("devmode = " + devmode);
  return {
    context: __dirname,
    entry: "src/bootstrap",
    output: {
      path: path.join(__dirname, "dist"),
      filename: "js/bundle.[fullhash].js"
    },
    module: {
      rules: [
        {
          test: /\.(png|jpe?g|gif)$/i,
          use: [{
            loader: "url-loader",
            /*options: {
              limit: 100000
            }*/
          }]
        }
      ]
    },
    plugins: [
      new webpack.ProgressPlugin(),
      new CleanWebpackPlugin(),

      // inject js
      new HtmlWebpackPlugin({
        template: "index.html"
      }),

      // dojo
      new DojoWebpackPlugin({
        loaderConfig: require("./src/loaderConfig"),
        environment: { dojoRoot: "" },  // used at run time for non-packed resources (e.g. blank.gif)
        buildEnvironment: { dojoRoot: "node_modules" }, // used at build time
        locales: ["en"],
        noConsole: true
      }),

      // copy non-packed resources needed by the app to the release directory
      new CopyWebpackPlugin({
        patterns: [{
	  context: "node_modules",
	  from: "dojo/resources/blank.gif",
	  to: "dojo/resources"
        }]
      }),

      // for plugins registered after the dojo-webpack-plugin, data.request has been normalized and
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
    mode: devmode ? 'development' : 'production',
    optimization: {
      moduleIds: 'natural',
      splitChunks: false,
      minimize: !devmode,
      minimizer: devmode ? [] : [new TerserPlugin({
      terserOptions: {
        format: {
          comments: false,
        },
      },
      extractComments: false,
      })]
    },
    performance: { hints: false },
    devtool: "hidden-source-map"
  };
};
