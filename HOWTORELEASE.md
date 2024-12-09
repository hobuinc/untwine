---
author: |
 Howard Butler
contact: |
  howard@hobu.co
date: 12/09/2024
title: Steps for Making a Untwine Release
---

# Release Process

This document describes the process for releasing a new version of Untwine.

1)  Set version numbers. The only place you need to set the Untwine version
    number is in the project() function of CMakeLists.txt.

    - CMakeLists.txt

      >     project(Untwine VERSION 1.5.0 LANGUAGES CXX )


2)  Ensure CI is ✅


3)  Tag the release and push the tag to GitHub. If the tag name is
    `x.x.x`, a draft release will be pushed to <https://github.com/hobuinc/Untwine/releases>
    >     git tag 1.5.0
    >     git push origin 1.5.0

4)  Write and update the [draft release notes.](https://github.com/hobuinc/Untwine/releases).
    The safest way to do this is to go though all the commits since the last release and reference
    any changes worthy of the release notes. See previous release notes for a template
    and a starting point. If you find issues after making the release branch,
    add them to the release notes.

5) Update Conda package

   - Conda Forge machinery should detect and build a new package for
     Untwine once the release tag is 'Published'.
