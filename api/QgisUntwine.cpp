#include <iostream>
#ifndef _WIN32
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#else
#include <Windows.h>
#endif

#include "QgisUntwine.hpp"

namespace untwine
{

QgisUntwine::QgisUntwine(const std::string& untwinePath) : m_path(untwinePath), m_running(false),
    m_percent(0)
{}

bool QgisUntwine::start(const StringList& files, const std::string& outputDir,
    const Options& argOptions)
{
    if (m_running)
        return false;

    Options options(argOptions);
    if (files.size() == 0 || outputDir.empty())
        return false;

    std::string s;
    for (auto ti = files.begin(); ti != files.end(); ++ti)
    {
        s += *ti;
        if (ti + 1 != files.end())
            s += ", ";
    }
    options.push_back({"files", s});
    options.push_back({"output_dir", outputDir});
#ifdef _WIN32
    HANDLE handle[2];
    SECURITY_ATTRIBUTES pipeAttr;
    pipeAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    pipeAttr.bInheritHandle = TRUE;
    pipeAttr.lpSecurityDescriptor = NULL;

    CreatePipe(&handle[0], &handle[1], &pipeAttr, 0);

    // Set the read end to no-wait.
    DWORD mode = PIPE_NOWAIT;
    SetNamedPipeHandleState(handle[0], &mode, NULL, NULL);

    size_t xhandle = reinterpret_cast<size_t>(handle[1]);
    options.push_back({"progress_fd", std::to_string(xhandle)});
    std::string cmdline;
    cmdline += m_path + " ";
    for (const Option& op : options)
        cmdline += "--" + op.first + " \"" + op.second + "\" ";

    PROCESS_INFORMATION processInfo;
    STARTUPINFO startupInfo;

    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    /**
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    **/

    std::vector<char> ncCmdline(cmdline.begin(), cmdline.end());
    ncCmdline.push_back((char)0);
    bool ok = CreateProcessA(m_path.c_str(), ncCmdline.data(),
        NULL, /* process attributes */
        NULL, /* thread attributes */
        TRUE, /* inherit handles */
        CREATE_NO_WINDOW, /* creation flags */
        NULL, /* environment */
        NULL, /* current directory */
        &startupInfo, /* startup info */
        &processInfo /* process information */
    );
    if (ok)
    {
        m_pid = processInfo.hProcess;
        m_progressFd = handle[0];
        m_running = true;
    }
    return ok;
#else
    int fd[2];
    int ret = ::pipe(fd);

    m_pid = ::fork();

    // Child
    if (m_pid == 0)
    {
        // Close file descriptors other than the stdin/out/err and our pipe.
        // There may be more open than FD_SETSIZE, but meh.
        for (int i = STDERR_FILENO + 1; i < FD_SETSIZE; ++i)
            if (i != fd[1])
                close(i);

        // Add the FD for the progress output
        options.push_back({"progress_fd", std::to_string(fd[1])});

        for (Option& op : options)
            op.first = "--" + op.first;

        std::vector<const char *> argv;
        argv.push_back(m_path.data());
        for (const Option& op : options)
        {
            argv.push_back(op.first.data());
            argv.push_back(op.second.data());
        }
        argv.push_back(nullptr);
        if (::execv(m_path.data(), const_cast<char *const *>(argv.data())) != 0)
        {
            std::cerr << "Couldn't start untwine '" << m_path << "'.\n";
            exit(-1);
        }
    }
    // Parent
    else
    {
        close(fd[1]);
        m_progressFd = fd[0];
        // Don't block attempting to read progress.
        ::fcntl(m_progressFd, F_SETFL, O_NONBLOCK);
        m_running = true;
    }
    return true;
#endif
}

bool QgisUntwine::stop()
{
    if (!m_running)
        return false;
#ifdef _WIN32
    TerminateProcess(m_pid, 1);
    WaitForSingleObject(m_pid, INFINITE);
    childStopped();
#else
    ::kill(m_pid, SIGINT);
    (void)waitpid(m_pid, nullptr, 0);
#endif
    m_pid = 0;
    return true;
}

// Called when the child has stopped.
void QgisUntwine::childStopped()
{
    m_running = false;
#ifdef _WIN32
    CloseHandle(m_progressFd);
    CloseHandle(m_pid);
#endif
}

bool QgisUntwine::running()
{
#ifdef _WIN32
    if (m_running && WaitForSingleObject(m_pid, 0) != WAIT_TIMEOUT)
        childStopped();
#else
    if (m_running && (::waitpid(m_pid, nullptr, WNOHANG) != 0))
        childStopped();
#endif
    return m_running;
}

int QgisUntwine::progressPercent() const
{
    readPipe();

    return m_percent;
}

std::string QgisUntwine::progressMessage() const
{
    readPipe();

    return m_progressMsg;
}

namespace
{

#ifndef _WIN32
uint32_t readString(int fd, std::string& s)
{
    uint32_t ssize;

    // Loop while there's nothing to read.  Generally this shouldn't loop.
    while (true)
    {
        ssize_t numRead = read(fd, &ssize, sizeof(ssize));
        if (numRead == -1 && errno != EAGAIN)
            continue;
        else if (numRead == sizeof(ssize))
            break;
        else
            return -1; // Shouldn't happen.
    }

    // Loop reading string
    char buf[80];
    std::string t;
    while (ssize)
    {
        ssize_t toRead = (std::min)((size_t)ssize, sizeof(buf));
        ssize_t numRead = read(fd, buf, toRead);
        if (numRead == 0 || (numRead == -1 && errno != EAGAIN))
            return -1;
        if (numRead > 0)
        {
            ssize -= numRead;
            t += std::string(buf, numRead);
        }
    }
    s = std::move(t);
    return 0;
}
#else
int readString(HANDLE h, std::string& s)
{
    uint32_t ssize;

    // Loop while there's nothing to read.  Generally this shouldn't loop.
    while (true)
    {
        DWORD numRead;
        bool ok = ReadFile(h, &ssize, sizeof(ssize), &numRead, NULL);
        // EOF or nothing to read.
        if (numRead == 0 && GetLastError() == ERROR_NO_DATA)
            continue;
        else if (numRead == sizeof(ssize))
            break;
        else
            return -1;
    }

    // Loop reading string
    char buf[80];
    std::string t;
    while (ssize)
    {
        DWORD numRead;
        DWORD toRead = (std::min)((size_t)ssize, sizeof(buf));
        ReadFile(h, buf, toRead, &numRead, NULL);
        if (numRead == 0 && GetLastError() == ERROR_NO_DATA)
            continue;
        if (numRead <= 0)
            return -1;
        ssize -= numRead;
        t += std::string(buf, numRead);
    }
    s = std::move(t);
    return 0;
}
#endif

} // unnamed namespace

#ifndef _WIN32
void QgisUntwine::readPipe() const
{
    // Read messages until the pipe has been drained.
    while (true)
    {
        ssize_t size = read(m_progressFd, &m_percent, sizeof(m_percent));
        // If we didn't read the full size, just return.
        if (size != sizeof(m_percent))
            return;

        // Read the string, waiting as necessary.
        if (readString(m_progressFd, m_progressMsg) != 0)
            break;
    }
}
#else
void QgisUntwine::readPipe() const
{
    // Read messages until the pipe has been drained.
    while (true)
    {
        DWORD numRead;
        ReadFile(m_progressFd, &m_percent, sizeof(m_percent), &numRead, NULL);
        if (numRead != sizeof(m_percent))
            return;

        // Read the string, waiting as necessary.
        if (readString(m_progressFd, m_progressMsg) != 0)
            break;
    }
}
#endif

} // namespace untwine
