# ducts
A PSR-7 compliant middleware dispatcher for Hack/HHVM.

It's the same idea as [Connect](https://github.com/senchalabs/connect), [Relay](https://github.com/relayphp/Relay.Relay), [Stratigility](https://github.com/zendframework/zend-stratigility), [Middleman](https://github.com/mindplay-dk/middleman), etc., but in strict mode Hack.

## Installation

You can install this library using Composer:

```console
$ composer require appertly/ducts
```

* The master branch (version 0.x) of this project requires HHVM 3.12 and has no dependencies.

## Compliance

Releases of this library will conform to [Semantic Versioning](http://semver.org).

Our code is intended to comply with [PSR-1](http://www.php-fig.org/psr/psr-1/), [PSR-2](http://www.php-fig.org/psr/psr-2/), and [PSR-4](http://www.php-fig.org/psr/psr-4/). If you find any issues related to standards compliance, please send a pull request!

## Layers and Dispatching

The idea here is that you supply a list of functions which:
1. Accept a request, a response, and a next function
2. Possibly modify the request and response
3. Possibly invoke the next function, which returns a response
4. Possibly modify the returned response
5. Return the response

```hack
use Psr\Http\Message\ServerRequestInterface as Request;
use Psr\Http\Message\ResponseInterface as Response;

function foobar(Request $req, Response $res, (function (Request,Response): Response) $next): Response
{
    // 2. possibly modify the request and response
    // 3. invoke next
    $res = $next($req, $res);
    // 4. possibly modify response
    // 5.
    return $res;
}
```

Layers end up interacting like this:
* → First layer
  * → Second layer
    * → Third layer
    * ← Third layer
  * ← Second layer
* ← First layer

### Dispatch a Request

```hack
use Ducts\Runner;

/**
 * @var Psr\Http\Message\RequestInterface $request The PSR-7 HTTP request
 * @var Psr\Http\Message\ResponseInterface $response The PSR-7 HTTP response
 */

// any Traversable will do. Use an array, Vector, etc.
$runner = new Runner([
    // you can use closures
    function(Request $req, Response $res, (function (Request,Response): Response) $next): Response {
        $req = $req->withAttribute('foo', 'bar');
        return $next($req, $res);
    },
    // or lambdas
    ($req, $res, $next) ==> $next($req, $res)->withHeader('X-Ducts', 'Yes')
    // or objects with an __invoke method
    new MyInvokableThing(),
    // or Hack-style callables
    fun('myLayer'),
    class_meth('MyLayer', 'staticMethod'),
    inst_meth($myObject, 'layer')
]);

$response = $runner->run($request, $response);

// Runner::run can be called multiple times if needed
$anotherResponse = $runner->run($request, $response);
```

You can also make use of the `ResolvingRunner` if you'd like to look up the layer callable.

```hack
// a function to look up names
$resolver = function(mixed $name): (function (Request,Response,(function(Request,Response): Response)): Response) {
    // look up your callable thing in a dependency injection container
    $container = MyClass::getSomeContainer();
    return $container->get($name);
};

// Items in this traversable can be a combination of functions or items to resolve
$runner = new ResolvingRunner([
    ($req, $res, $next) ==> $next($req, $res)->withHeader('X-Ducts', 'Yes'),
    'MyClassName',
    'AnotherClass'
], $resolver);
```

### A Runner as a Layer

You can use a `Runner` as a layer by calling `$runner->layer()`!
For example, you declare `Runner` `$bar` to be the second layer of three in `Runner` `$foo`.
If `$bar` has three layers, it will participate in `$foo` like this:

* → `$foo` layer 1
  * → `$bar` layer 1
    * → `$bar` layer 2
      * → `$bar` layer 3
        * → `$foo` layer 3
        * ← `$foo` layer 3
      * ← `$bar` layer 3
    * ← `$bar` layer 2
  * ← `$bar` layer 1
* ← `$foo` layer 1
