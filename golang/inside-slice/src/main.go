package main

import (
	"fmt"
)

func main() {
	a := make([]int, 10, 20)

	b := a[5:]
	println(len(b), cap(b))

	c := a[1:2:2]
	c = append(c, 4)
	fmt.Printf("%v,%v", a, c)

}
