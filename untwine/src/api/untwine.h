/*****************************************************************************
 *   Copyright (c) 2020, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#ifndef UNTWINE_H
#define UNTWINE_H

#ifdef UNTWINE_STATIC
#  define UNTWINE_EXPORT
#else
#  if defined _WIN32 || defined __CYGWIN__
#    ifdef UNTWINE_EXPORTS
#      ifdef __GNUC__
#        define UNTWINE_EXPORT __attribute__ ((dllexport))
#      else
#        define UNTWINE_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    else
#      ifdef __GNUC__
#        define UNTWINE_EXPORT __attribute__ ((dllimport))
#      else
#        define UNTWINE_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    endif
#  else
#    if __GNUC__ >= 4
#      define UNTWINE_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define UNTWINE_EXPORT
#    endif
#  endif
#endif

#include <functional>
#include <string>

/**
 * Log levels
 */
namespace Untwine {

     /**
     * Returns untwine version (x.y.z)
     */
    UNTWINE_EXPORT std::string version();

    enum LogLevel
    {
      Error,
      Warn,
      Info,
      Debug
    };

    typedef std::function<void(LogLevel logLevel, const std::string& message)> LoggerCallbackFunction;

    /**
     * Sets custom callback for logging output
     */
    UNTWINE_EXPORT void SetLoggerCallback( LoggerCallbackFunction callback );

    /**
     * Sets maximum log level (verbosity)
     *
     * By default logger outputs errors only.
     * Log levels (low to high): Error, Warn, Info, Debug
     * For example, if LogLevel is set to Warn, logger outputs errors and warnings.
     */
    UNTWINE_EXPORT void SetLogVerbosity( LogLevel verbosity );

    UNTWINE_EXPORT class Feedback
    {
    public:
        enum Status {
            Canceled = 0,
            Running,
            Finished, //Success
            Failed
        };

        //! Request cancellation
        void requestCancellation();

        //! Returns status of the current operation
        Status status() const;

        //! Returns progress 0-100
        int progress() const;
    }

    /**
     * Starts a new preflight step from the folder of files or a single point cloud file
     * \param uri single point cloud file readable by PDAL
     * \param outputDir folder to write point cloud buckets
     * \param options string map defining options/flags, empty for all defaults
     */
    UNTWINE_EXPORT Feedback PreFlightClustering(
        const std::string& uri,
        const std::string& outputDir,
        const std::map<std::string, std::string>& options
    );

    /**
     * Starts a new bottom-up indexing
     * \param inputDir input directory from UNTWINE_PreFlight
     * \param outputDir folder to write EPT files
     * \param options string map defining options/flags, empty for all defaults
     */
    UNTWINE_EXPORT Feedback BottomUpIndexing(
        const char *inputDir,
        const char *outputDir,
        const std::map<std::string, std::string>& options
    );

} // namespace Untwine

#endif // UNTWINE_H