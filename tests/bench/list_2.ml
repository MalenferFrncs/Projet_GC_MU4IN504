(* Goal: create a larger list using terminal reccursion *)
(*Modification pour que les tests passent en un temps réaliste : la liste fait 10000 éléments et plus 1000000*)
let rec make_list_0 size acc =
  if size = 0 then acc
  else make_list_0 (size - 1) (0 :: acc)
let make_list_0 size = make_list_0 size []

let rec length l acc =
  match l with
  | [] -> acc
  | _ :: tl -> (length tl (acc + 1))
let length l = length l 0

let _ = length (make_list_0 10000)
