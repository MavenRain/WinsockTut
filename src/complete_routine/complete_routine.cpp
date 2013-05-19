
#include "complete_routine.h"
#include <map>


static std::map<int, socket_data*> g_sockList;



socket_data* alloc_data(SOCKET sockfd)
{
    socket_data* data = NULL;
    try
    {
        data = new socket_data();
    }
    catch (std::bad_alloc&)
    {
        _tprintf(_T("allocate socket data failed.\n"));
        return NULL;
    }
        
    // winsock didn't use hEvent in complete routine model
    data->overlap_.hEvent = (WSAEVENT)data; 
    data->socket_ = sockfd;
    data->wsabuf_.buf = data->databuf_;
    data->wsabuf_.len = sizeof(data->databuf_);
    
    g_sockList[sockfd] = data;

    return data;
}


void free_data(socket_data* data)
{
    if (data)
    {
         _tprintf(_T("socket %d closed at %s.\n"), data->socket_, Now().data());
        closesocket(data->socket_);
        g_sockList.erase(data->socket_);
        delete data;
    }
}


bool post_recv_request(socket_data* data)
{
    assert(data);
    DWORD flags = 0;
    DWORD recv_bytes = 0;
    int error = WSARecv(data->socket_, &data->wsabuf_, 1, &recv_bytes, &flags, 
        &data->overlap_, recv_complete);
    if (error == 0 || 
        (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
    {
        return true;
    }
    else
    {
        _tprintf(_T("WSARecv() failed [%d], %s"), data->socket_, LAST_ERROR_MSG);
        free_data(data);
        return false;
    }
}


//////////////////////////////////////////////////////////////////////////

void CALLBACK recv_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap, 
                            DWORD flags)
{
    assert(overlap);
    socket_data* data = (socket_data*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        free_data(data);
        return ;
    }

    // send data back
    memset(&data->overlap_, 0, sizeof(data->overlap_));
    data->overlap_.hEvent = (WSAEVENT)data;
    data->wsabuf_.len = bytes_transferred;
    DWORD bytes_send = 0;
    error = WSASend(data->socket_, &data->wsabuf_, 1, &bytes_send, flags, 
                &data->overlap_, send_complete);
    if (error == 0 || 
        (error == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
    {
        // succeed
        return ;
    }
    else 
    {
        free_data(data);
    }
}


void CALLBACK send_complete(DWORD error, 
                            DWORD bytes_transferred, 
                            WSAOVERLAPPED* overlap,
                            DWORD /*flags*/)
{
    assert(overlap);
    socket_data* data = (socket_data*)overlap->hEvent;
    if (error || bytes_transferred == 0)
    {
        free_data(data);
        return ;
    }

    // post another recv
    data->wsabuf_.len = sizeof(data->databuf_);
    post_recv_request(data);
}