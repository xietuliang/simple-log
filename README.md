# simple-log
调用接口可以参考：

int main() {
    auto log = base::Log::Instance();
    log->registerInterface(log);
    base::Log::OUT(base::Log::Debug) << 2345;
    base::Log::OUT(base::Log::Info) << 12.34;
    base::Log::OUT(base::Log::Warn) << "hello";
    base::Log::OUT(base::Log::Error) << 'h';
	return 0;
}
