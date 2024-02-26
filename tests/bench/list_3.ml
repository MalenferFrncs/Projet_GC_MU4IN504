(* Goal: create a few temporary lists that can be freed *)

let rec make_list_0 size acc =
  if size = 0 then acc
  else make_list_0 (size - 1) (0 :: acc)
let make_list_0 size = make_list_0 size []

let rec map f l aux =
  match l with
  | [] -> aux
  | hd :: tl -> map f tl ((f hd) :: aux)
let map f l = map f l []

let rec length l acc =
  match l with
  | [] -> acc
  | _ :: tl -> (length tl (acc + 1))
let length l = length l 0

let add_1 x = x + 1

let map_add_1 = map add_1

let _ = length (map_add_1 (map_add_1 (map_add_1 (map_add_1 (make_list_0 100000)))))
