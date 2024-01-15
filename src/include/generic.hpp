#ifndef QLIBS_GENERIC
#define QLIBS_GENERIC

#include <include/qlibs_types.hpp>

/*!
 * @file generic.hpp
 * @author J. Camilo Gomez C.
 * @version 1.01
 * @note This file is part of the qLibs-cpp distribution.
 * @brief Type-Generic utilities
 **/


/**
* @brief The qLibs++ library namespace.
*/
namespace qlibs {

    /**
    * @brief The generic namespace.
    */
    namespace generic {
        /** @addtogroup  qtypegeneric Type-generic utilities
        * @brief Type-generic utilities
        * @{
        */


        /*! @cond  */
        using compareFcn_t = int (*)( const void *, const void *, void * );
        using forEachFcn_t = int (* const)( int, void *, void * );
        /*! @endcond  */

        /**
        * @brief Swaps the data pointed by @a x and @a y
        * @param[in,out] x Pointer to data to be swapped.
        * @param[in,out] y Pointer to data to be swapped.
        * @param[in] n The size of the data to be swapped.
        * @return none.
        */
        void swap( void * const x,
                   void * const y,
                   size_t n ) noexcept;

        /**
        * @brief Sorts the given array pointed to by @a pbase in ascending order.
        * The array contains @a n elements of @a size bytes. Function pointed to
        * by @a cmp is used for object comparison.
        * @remark This algorithm uses a non-recursive variant of the quicksort
        * algorithm.
        * @param[in,out] pbase Pointer to the array to sort.
        * @param[in] n Number of elements in the array.
        * @param[in] size size of each element in the array in bytes
        * @param[in] cmp Comparison function which returns ​a negative integer value
        * if the first argument is less than the second, a positive integer value
        * if the first argument is greater than the second and zero if the arguments
        * are equivalent.
        * The signature of the comparison function should be equivalent to the
        * following:
        * @code{.c}
        *  int cmp( const void *a, const void *b, void *arg );
        * @endcode
        * The function must not modify the objects passed to it and must return
        * consistent results when called for the same objects, regardless of their
        * positions in the array.
        * @param[in] arg Additional information (e.g., collating sequence), passed
        * to @c cmp as the third argument
        * @return none.
        */
        void sort( void * const pbase,
                   size_t n,
                   size_t size,
                   compareFcn_t cmp,
                   void *arg = nullptr ) noexcept;

        /**
        * @brief Reverse the given array pointed to by @a pbase.
        * Operation takes place on the portion of the array that starts at position
        * @a init to position @a end.
        * @param[in,out] pbase Pointer to the array to reverse.
        * @param[in] size Size of each element in the array in bytes
        * @param[in] init Position of the first element.
        * @param[in] end Position of the last element.
        * @return none.
        */
        void reverse( void * const pbase,
                      const size_t size,
                      const size_t init,
                      const size_t end ) noexcept;

        /**
        * @brief Rotates @a k elements of the array pointed to by @a pbase. The array
        * contains @a n elements of @a size bytes. Rotation direction is determined
        * by the sign of @a k, the means a positive value performs a right-rotation
        * and a negative value a left-rotation.
        * @param[in,out] pbase Pointer to the array to reverse.
        * @param[in] size Size of each element in the array in bytes
        * @param[in] n Number of elements in the array.
        * @param[in] k Positions to rotate.
        * @return none.
        */
        void rotate( void * const pbase,
                     const size_t size,
                     const size_t n,
                     const int k ) noexcept;

        /**
        * @brief Set the data pointed by @a ref to every element of the array
        * pointed by @a pbase. The array contains @a n elements of @a size bytes
        * @param[in,out] pbase Pointer to the array to reverse.
        * @param[in] size Size of each element in the array in bytes
        * @param[in] n Number of elements in the array.
        * @param[in] ref Pointer to the value to be set.
        * @return This function returns a pointer to the memory area @a pbase
        */
        void* set( void * const pbase,
                   const size_t size,
                   const size_t n,
                   const void * const ref ) noexcept;

        /**
        * @brief Performs a linear search over an array of @a n elements
        * pointed to by @a pbase for an element that matches the object pointed to
        * by @a key.
        * The size of each element is specified by @a size. The array contents
        * should be sorted in ascending order according to the @a compar
        * comparison function. This routine should take two arguments pointing to
        * the @a key and to an array element, in that order, and should return an
        * integer less than, equal to, or greater than zero if the @a key object
        * is respectively less than, matching, or greater than the array element.
        * @param[in] key This is the pointer to the object that serves as key for
        * the search, type-casted as a @c void*.
        * @param[in] pbase This is the pointer to the first object of the array
        * where the search is performed, type-casted as a @c void*.
        * @param[in] n This is the number of elements in the array pointed by
        * @a pbase.
        * @param[in] size This is the size in bytes of each element in the array.
        * @param[in] compar This is the function that compares two elements.
        * The signature of the comparison function should be equivalent to the
        * following:
        * @code{.c}
        *  int compar( const void *a, const void *b, void *arg );
        * @endcode
        * Comparison function which returns ​a negative integer value if the first
        * argument is less than the second, a positive integer value if the first
        * argument is greater than the second and zero if the arguments are
        * equivalent. @a key is passed as the first argument, an element from the
        * array as the second.
        * The function must not modify the objects passed to it and must return
        * consistent results when called for the same objects, regardless of their
        * positions in the array.
        * @param[in] arg Additional information (e.g., collating sequence), passed
        * to @a compar as the third argument
        * @return This function returns a pointer to an entry in the array that
        * matches the search key. If key is not found, a @c nullptr pointer is returned.
        */
        void* lSearch( const void *key,
                       const void *pbase,
                       const size_t n,
                       const size_t size,
                       compareFcn_t compar,
                       void *arg = nullptr ) noexcept;

        /**
        * @brief Performs a binary search over an array of @a n elements
        * pointed to by @a pbase for an element that matches the object pointed to
        * by @a key.
        * The size of each element is specified by @a size. The array contents
        * should be sorted in ascending order according to the @a compar
        * comparison function. This routine should take two arguments pointing to
        * the @a key and to an array element, in that order, and should return an
        * integer less than, equal to, or greater than zero if the @a key object
        * is respectively less than, matching, or greater than the array element.
        * @param[in] key This is the pointer to the object that serves as key for
        * the search, type-casted as a @c void*.
        * @param[in] pbase This is the pointer to the first object of the array
        * where the search is performed, type-casted as a @c void*.
        * @param[in] n This is the number of elements in the array pointed by
        * @a base.
        * @param[in] size This is the size in bytes of each element in the array.
        * @param[in] compar This is the function that compares two elements.
        * The signature of the comparison function should be equivalent to the
        * following:
        * @code{.c}
        *  int compar( const void *a, const void *b, void *arg );
        * @endcode
        * Comparison function which returns ​a negative integer value if the first
        * argument is less than the second, a positive integer value if the first
        * argument is greater than the second and zero if the arguments are
        * equivalent. @a key is passed as the first argument, an element from the
        * array as the second.
        * The function must not modify the objects passed to it and must return
        * consistent results when called for the same objects, regardless of their
        * positions in the array.
        * @param[in] arg Additional information (e.g., collating sequence), passed
        * to @a compar as the third argument
        * @return This function returns a pointer to an entry in the array that
        * matches the search key. If key is not found, a @c nullptr pointer is returned.
        */
        void* bSearch( const void *key,
                       const void *pbase,
                       const size_t n,
                       const size_t size,
                       compareFcn_t compar,
                       void *arg = nullptr ) noexcept;

        /**
        * @brief Iterates @a n elements of the array pointed to by @a pbase.
        * The size of each element is specified by @a size. Every element should be
        * handled by function @a f. The iteration loop can be aborted by returning 1.
        * @param[in] pbase This is the pointer to the first object of the array
        * type-casted as a @c void*.
        * @param[in] n This is the number of elements in the array pointed by
        * @a pbase.
        * @param[in] size This is the size in bytes of each element in the array.
        * @param[in] f The function that will handle each element of the array
        * The signature of this handling function should be equivalent to the
        * following:
        * @code{.c}
        * int iterFcn( int i, void *element, void *arg );
        * @endcode
        * The argument @a i is used to keep track of the iteration in which the loop
        * is.
        * - <tt>i < 0</tt> Loop its a about to start (pre-loop invocation). The
        * @a element argument at this stage its passed as @c NULL and should not be
        * dereferenced.
        * - <tt>[ 0 <= i < n ]</tt> Loop its iterating and the @a element argument
        * is pointing to the array at index @a i.
        * - <tt>i == n</tt> Loop has ended (pos-loop invocation). The @a element
        * argument at this stage its passed as @c NULL and should not be dereferenced.
        * @param[in] dir Pass @c true to iterate the array backwards.
        * @param[in] arg Additional information (e.g., collating sequence), passed
        * to @a f as the third argument
        * @return This function returns 1 if the iteration loop is aborted otherwise
        * returns 0.
        */
        int forEach( void *pbase,
                     const size_t size,
                     const size_t n,
                     forEachFcn_t f,
                     const bool dir = false,
                     void *arg = nullptr ) noexcept;

        /** @}*/
    }
}


#endif /*QLIBS_GENERIC*/