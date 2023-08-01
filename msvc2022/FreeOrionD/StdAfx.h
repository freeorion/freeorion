
#pragma once

// We include all external headers used in any of the header files,
// plus external headers used in at least five .cpp files.

// https://hownot2code.com/2016/08/16/stdafx-h/ 

// ----------------
// includes from .h

// Common
#include <array>
#include <bitset>
#include <chrono>
#include <cmath>
#include <ctime>
#include <deque>
#include <future>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/any.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/optional/optional.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/python/detail/destroy.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/version.hpp>
#include <boost/shared_array.hpp>
#include <boost/signals2/optional_last_value.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid.hpp>

// FreeOrionD
#include <queue>

#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/mpl/list.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/python.hpp>
#include <boost/python/dict.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/deferral.hpp>
#include <boost/statechart/event.hpp>
#include <boost/statechart/in_state_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>

#ifdef _MSC_VER
// Note: This is a workaround for Visual C++ non-conformant pre-processor
// handling of empty macro arguments.
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/is_empty_variadic.hpp>
#endif
