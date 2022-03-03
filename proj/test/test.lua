logger.debug("hello world")
logger.warning("warning")
logger.error("error")

local my_service={
on_session_recv_cmd=function(session,msg)

end,
on_session_disconnect=function(session)
end
}

local ret=service.register(100,my_service)
print(ret)

scheduler.schedule(function() print("shceduler")end,1000,-1,5000)