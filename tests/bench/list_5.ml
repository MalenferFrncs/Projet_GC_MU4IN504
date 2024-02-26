(* Goal: create some small lists *)

let make_list x y = x :: y :: []

let rec make_n_list x y n =
  if n = 0 then make_list x y
  else let _ = make_list x y in
       make_n_list x y (n-1)

let _ = make_n_list 1 5 10
