file(READ "${SOURCE_DIR}/include/opendht/value.h" content)
string(REPLACE
    "        template<typename Functor>
        Filter(Functor f)
            : std::function<bool(const Value&)>::function(f)
        {}"
    "        template<typename Functor>
        Filter(Functor f)
        {
            std::function<bool(const Value&)> tmp(std::move(f));
            static_cast<std::function<bool(const Value&)>&>(*this) = std::move(tmp);
        }"
    content "${content}")
file(WRITE "${SOURCE_DIR}/include/opendht/value.h" "${content}")
