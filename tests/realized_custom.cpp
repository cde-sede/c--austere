// expect: exit 30
#include "2-vocabulary/realized.hpp"
#include "2-vocabulary/uncertain.hpp"

class even final : public ca::uncertain<even, int> {

	public:

		explicit even(const int value) noexcept : _value{value} {}

	private:

		friend class ca::uncertain<even, int>;

		ca::realized<int> _value;

		constexpr auto _realizable(void) const noexcept -> bool { return *_value % 2 == 0; }
		constexpr auto _realized_ref(void) const& noexcept -> const ca::realized<int>& { return _value; }
		template <typename F>
		constexpr auto _absent(F& f) const noexcept -> decltype(auto) { return f(); }
};

auto main(void) -> int {
	const even yes{20};
	const even no{7};
	if (!yes.holds() || no.holds())
		return 1;
	return *yes + no.match([](const ca::realized<int>& v) { return *v; }, [] { return 10; });
}
