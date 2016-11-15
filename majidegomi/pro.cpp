#include <utility>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <boost/variant.hpp>

namespace pro {
  struct void_value_t;
  struct int_value_t;
  struct var_t;
  struct lambda_t;
  struct closure_t;
  struct apply_t;

  using expression = boost::variant <
    std::shared_ptr <void_value_t>
  , std::shared_ptr <int_value_t>
  , std::shared_ptr <var_t>
  , std::shared_ptr <lambda_t>
  , std::shared_ptr <closure_t>
  , std::shared_ptr <apply_t>
  >;

  using environ_t = std::map <std::string , expression>;

  namespace detail {
    struct show_f {
      constexpr show_f () noexcept {}

      template <typename T>
      auto operator () (const T & p) const -> std::string {
        return show (p);
      }
    };

    struct eval_f {
      const environ_t & env;

      constexpr eval_f (const environ_t & e)
        : env {e} {}

      template <typename T>
      auto operator () (const T & p) const -> expression {
        return eval (env , p);
      }
    };

    struct pattern_match_f {
      const expression & expr;
      environ_t * env;

      pattern_match_f (const expression & e , environ_t * env_)
        : expr {e}
        , env {env_} {}

      template <typename T>
      auto operator () (const T & p) const -> bool {
        return pattern_match (p , expr , * env);
      }
    };
  }

  // ラムダ式でやろうとするとめっちゃエラーが出る(´・ω・｀)
  inline auto show (const expression & p) {
    return boost::apply_visitor (detail::show_f {} , p);
  }

  inline auto eval (const environ_t & env , const expression & p) {
    return boost::apply_visitor (detail::eval_f {env} , p);
  }

  inline auto pattern_match (const expression & p , const expression & e , environ_t & env) {
    return boost::apply_visitor (detail::pattern_match_f {e , & env} , p);
  }


  struct void_value_t {
    constexpr void_value_t () noexcept {}
  };

  inline auto Void () {
    return std::make_shared <void_value_t> ();
  }

  inline auto show (const std::shared_ptr <void_value_t> &) {
    return "()";
  }

  inline auto eval (const environ_t &, const std::shared_ptr <void_value_t> & p) {
    return p;
  }

  inline auto pattern_match (const std::shared_ptr <void_value_t> & , const expression & e , environ_t &) {
    try {
      // ignore return value
      boost::get <std::shared_ptr <void_value_t>> (e);
      return true;
    }
    catch (const boost::bad_get &) {
      return false;
    }
  }


  struct int_value_t {
    using value_type = std::int64_t;

    value_type data;

    constexpr int_value_t (value_type && d)
      : data (std::move (d)) {}
  };

  inline auto Int (int_value_t::value_type && d) {
    return std::make_shared <int_value_t> (std::move (d));
  }

  inline auto show (const std::shared_ptr <int_value_t> & p) {
    std::stringstream ss;
    ss << p -> data;
    return ss.str ();
  }

  inline auto eval (const environ_t &, const std::shared_ptr <int_value_t> & p) {
    return p;
  }

  inline auto pattern_match (const std::shared_ptr <int_value_t> & p , const expression & e , environ_t &) {
    try {
      auto ep = boost::get <std::shared_ptr <int_value_t>> (e);
      return (p -> data == ep -> data);
    }
    catch (const boost::bad_get &) {
      return false;
    }
  }


  struct var_t {
    using name_type = std::string;

    name_type name;

    var_t (name_type && n)
      : name {std::move (n)} {}
  };

  inline auto Var (var_t::name_type && n) {
    return std::make_shared <var_t> (std::move (n));
  }

  inline auto show (const std::shared_ptr <var_t> & v) {
    std::stringstream ss;
    ss << "var:" << v -> name;
    return ss.str ();
  }


  inline auto eval (const environ_t & env, const std::shared_ptr <var_t> & p) {
    auto ite = env.find (p -> name);
    if (ite != env.end ()) {
      return eval (env , ite -> second);
    }
    std::stringstream ss;
    ss << p -> name << " is undefined.";
    throw std::runtime_error {ss.str ()};
  }

  inline auto pattern_match (const std::shared_ptr <var_t> & xp , const expression & e , environ_t & env) {
    env [xp -> name] = e;
    return true;
  }


  struct lambda_t {
    using arg_type = expression;
    using body_type = expression;

    arg_type arg;
    body_type body;

    lambda_t (arg_type && a , body_type && b)
      : arg {std::move (a)}
      , body {std::move (b)} {}
  };

  inline auto Lambda (lambda_t::arg_type && a , lambda_t::body_type && b) {
    return std::make_shared <lambda_t> (std::move (a) , std::move (b));
  }

  inline auto show (const std::shared_ptr <lambda_t> &) {
    return "this is lambda.";
  }

  inline auto eval (const environ_t & env , const std::shared_ptr <lambda_t> & p) {
    return std::make_shared <closure_t> (env , p);
  }

  inline auto pattern_match (const std::shared_ptr <lambda_t> & , const expression & , environ_t &) -> bool {
    throw std::runtime_error {"failed pattern match. lambda is not a constructor."};
  }


  struct closure_t {
    using lambda_type = std::shared_ptr <lambda_t>;

    environ_t environ;
    lambda_type lambda;

    closure_t (const environ_t & e , const lambda_type & l)
      : environ {e}
      , lambda {l} {}
  };

  inline auto show (const std::shared_ptr <closure_t> &) {
    return "this is closure.";
  }

  inline auto eval (const environ_t &, const std::shared_ptr <closure_t> & p) {
    return p;
  }

  inline auto pattern_match (const std::shared_ptr <closure_t> & , const expression & , environ_t &) -> bool {
    throw std::runtime_error {"failed pattern match. closure is not a constructor."};
  }


  struct apply_t {
    using func_type = expression;
    using expr_type = expression;

    func_type func;
    expr_type expr;

    apply_t (func_type && f , expr_type && e)
      : func {std::move (f)}
      , expr {std::move (e)} {}
  };

  inline auto Apply (apply_t::func_type && f , apply_t::expr_type && e) {
    return std::make_shared <apply_t> (std::move (f) , std::move (e));
  }

  inline auto Let (expression && a , expression && e , expression && b) {
    return Apply (Lambda (std::move (a) , std::move (b)) , std::move (e));
  }

  inline auto show (const std::shared_ptr <apply_t> &) {
    return "cannot show unevalated value.";
  }

  inline auto eval (const environ_t & env, const std::shared_ptr <apply_t> & p) {
    try {
      auto f = boost::get <std::shared_ptr <closure_t>> (eval (env , p -> func));
      environ_t new_env;
      if (pattern_match (f -> lambda -> arg , eval (env , p -> expr) , new_env)) {
        new_env.insert (f -> environ.begin () , f -> environ.end ());
        return eval (new_env , f -> lambda -> body);
      }
    }
    catch (const boost::bad_get &) {
      throw std::runtime_error {"the object <which is not a function> cannot apply."};
    }
    throw std::runtime_error {"failed pattern match."};
  }

  inline auto pattern_match (const std::shared_ptr <apply_t> & , const expression & , environ_t &) -> bool {
    throw std::runtime_error {"failed pattern match. function apply is not a constructor."};
  }
}

auto main () -> int {
  using namespace pro;
  // auto e = Apply (Lambda (Var ("x") , Var ("x")) , Int (3));
  // auto e = Apply (Lambda (Int (3) , Int (6)) , Int (3));
  // auto e = Apply (Lambda (Void () , Int (334)) , Void ());
  auto e =
  Let (Var ("x") , Int (1) ,
    Let (Var ("f") , Lambda (Var ("_") , Var ("x")) ,
      Let (Var ("x") , Int (10) ,
        Apply (Var ("f") , Var ("x"))
      )
    )
  );
  environ_t env;
  std::cout << show (eval (env , e)) << std::endl;
}
