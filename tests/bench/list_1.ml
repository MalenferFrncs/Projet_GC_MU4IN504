(* Goal: create a large list *)

let rec make_list_0 size =
  if size = 0 then []
  else 0 :: (make_list_0 (size - 1))

let rec length l =
  match l with
  | [] -> 0
  | _ :: tl -> 1 + (length tl)

let _ = length (make_list_0 10000)
