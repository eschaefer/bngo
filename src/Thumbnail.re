[@react.component]
let make = (~url=?) => {
  let artwork =
    switch (url) {
    | Some(url) => url
    | None => ""
    };

  let style =
    ReactDOMRe.Style.make(~background="url(" ++ artwork ++ ") center", ());

  <div className="w3 w4-l relative">
    <div className="aspect-ratio aspect-ratio--1x1">
      <div className="aspect-ratio--object cover" style />
    </div>
  </div>;
};