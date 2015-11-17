# Introduction #

This page describes the steps required to create a bdremote-ng release.

# Procedure #

  * Update Changelog.
  * Make sure that the version in header file and gen-release.sh match. The cmake config will abort configuration if they do not.
  * Create tag using `svn copy https://bdremote-ng.googlecode.com/svn/trunk https://bdremote-ng.googlecode.com/svn/tags/version_x_y -m "Added version_x_y tag."`.
  * Run `build/gen-release.sh`
  * Upload generated `.tar.bz2` file to project site.
  * Update list of releases on wiki.
  * Send e-mail to mailing list.
  * Update http://freshmeat.net entry.

# Release Candidate #

The above steps should also be used to create a RC, without updating the freshmeat.net entry.